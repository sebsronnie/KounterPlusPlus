// -----------------------------------------------------------------------------
//  Kounter++  |  Coding Camp II  -  Project 3
//  Team Foxtrot: Ronald Sebuhinja, Kelvin Maritim
//  File:    ofApp.cpp
//  Author:  Ronald K. Sebuhinja (shared file: change only by agreement)
//  Purpose: Event wiring between GUI, sources, vision pipeline and overlay.
// -----------------------------------------------------------------------------
#include "ofApp.h"

#include "app/CvBridge.h"
#include "sources/CameraSource.h"
#include "sources/ImageSource.h"
#include "sources/VideoFileSource.h"

namespace {
	const float kPanelWidth = 260.0f;

	bool hasExtension(const std::string& path, std::initializer_list<const char*> exts) {
		const std::string ext = ofToLower(ofFilePath::getFileExt(path));
		for (const char* e : exts) if (ext == e) return true;
		return false;
	}
}

// ------------------------------------------------------------------ lifecycle

void ofApp::setup() {
	ofSetWindowTitle("Kounter++ - Team Foxtrot");
	ofSetFrameRate(60);
	ofSetEscapeQuitsApp(false);                 // we use Esc to cancel typing
	ofBackground(20);

	panel.setup(10, 10);
	panel.onLoadImage      = [this] { chooseImageDialog(); };
	panel.onUseCamera      = [this] { openCamera(); };
	panel.onLoadVideo      = [this] { chooseVideoDialog(); };
	panel.onClearClasses   = [this] { detector.getClassifier().clear(); needsRun = true; say("Taught classes cleared"); };
	panel.onResetCounter   = [this] { detector.getTracker().reset(); say("Video counter reset"); };
	panel.onSaveScreenshot = [this] { saveScreenshot(); };
	panel.onExportCsv      = [this] { exportCsv(); };

	overlay.setup();
	layout();

	// Start with a bundled sample so the app never opens on an empty screen.
	openImage(ofToDataPath("images/sample_oranges_table.png"));
}

void ofApp::layout() {
	overlay.setViewport(ofRectangle(kPanelWidth + 20, 0, ofGetWidth() - kPanelWidth - 20, ofGetHeight()));
}

void ofApp::windowResized(int, int) { layout(); }

void ofApp::exit() {
	if (source) source->close();                // release the camera cleanly
}

// ------------------------------------------------------------------- per frame

void ofApp::update() {
	panel.applyTo(settings);                    // sliders -> settings, every frame
	if (settings != lastSettings) { needsRun = true; lastSettings = settings; }

	if (!source) return;
	source->update();

	if (source->isFrameNew()) {
		frame = CvBridge::toMat(source->getPixels());   // no copy
		CvBridge::toTexture(frame, frameTexture);
		needsRun = true;
	}
	if (needsRun && !frame.empty()) runDetection();
}

void ofApp::runDetection() {
	detector.process(frame, settings, source && source->isLive(), result);
	CvBridge::toTexture(result.mask, maskTexture);
	needsRun = false;
}

void ofApp::draw() {
	HudInfo hud;
	hud.sourceName = source ? source->getName() : "no source";
	hud.modeName   = result.modeName;
	hud.isVideo    = source && source->isLive();
	hud.teachMode  = teachMode;
	hud.showHelp   = showHelp;
	if (typing) hud.prompt = "Name for object " + ofToString(teachObjectIndex + 1) + ": " + labelBuffer + "_";
	if (ofGetElapsedTimef() < messageUntil) hud.message = message;

	overlay.draw(frameTexture, maskTexture, result, view, detector.getClassifier().getClasses(), hud);
	panel.draw();
}

// --------------------------------------------------------------------- sources

void ofApp::setSource(std::unique_ptr<FrameSource> next) {
	frame.release();                            // it wrapped the OLD source's pixels
	if (source) source->close();
	source = std::move(next);                   // the old source is destroyed here
	detector.getTracker().reset();
	needsRun = true;
}

void ofApp::openImage(const std::string& path) {
	auto img = std::make_unique<ImageSource>();
	if (!img->load(path)) { say("Could not load image"); return; }
	setSource(std::move(img));
	say("Loaded " + source->getName());
}

void ofApp::openCamera() {
	auto cam = std::make_unique<CameraSource>();
	if (!cam->open(0)) { say("No camera found"); return; }
	setSource(std::move(cam));
	say("Camera on - counting live");
}

void ofApp::openVideo(const std::string& path) {
	auto vid = std::make_unique<VideoFileSource>();
	if (!vid->load(path)) { say("Could not load video"); return; }
	setSource(std::move(vid));
	say("Playing " + source->getName());
}

void ofApp::openFile(const std::string& path) {
	if (hasExtension(path, {"mp4", "mov", "m4v", "avi", "mkv"})) openVideo(path);
	else openImage(path);
}

void ofApp::chooseImageDialog() {
	ofFileDialogResult r = ofSystemLoadDialog("Choose an image");
	if (r.bSuccess) openImage(r.getPath());
}

void ofApp::chooseVideoDialog() {
	ofFileDialogResult r = ofSystemLoadDialog("Choose a video");
	if (r.bSuccess) openVideo(r.getPath());
}

void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (dragInfo.files.empty()) return;
	const of::filesystem::path p = dragInfo.files[0];
	openFile(p.string());
}

// ----------------------------------------------------------------------- input

void ofApp::mousePressed(int x, int y, int button) {
	if (button != 0 || panel.isMouseOver(x, y)) return;   // the panel handles its own clicks
	int ix, iy;
	if (!overlay.screenToImage(x, y, ix, iy)) return;
	handleImageClick(ix, iy);
}

void ofApp::handleImageClick(int ix, int iy) {
	if (teachMode) {
		const DetectedObject* hit = ObjectDetector::objectAt(result, cv::Point(ix, iy));
		if (!hit) { say("No object there"); return; }
		teachObjectIndex = int(hit - &result.objects[0]);  // index of the hit object
		labelBuffer.clear();
		typing = true;
		return;
	}
	
	if (settings.mode == SegmentationMode::ColourPick && !frame.empty()) {
		detector.pickSeedColour(frame, cv::Point(ix, iy), settings);
		lastSettings = settings;                // the pick is not a slider change
		needsRun = true;
		say("Colour picked - adjust 'Colour tolerance'");
	}
	
	const DetectedObject* o = ObjectDetector::objectAt(result, cv::Point(ix, iy));
	if (!o) { say("No object clicked"); return; }
	
	say("area " + ofToString(int(o->area)) + " px"
		+ "   round " + ofToString(o->circularity, 2)
		+ "   aspect " + ofToString(o->aspectRatio, 2)
		+ "   solid " + ofToString(o->solidity, 2)
		+ (o->label.empty() ? "" : "   " + o->label));
}

void ofApp::finishTeaching() {
	typing = false;
	if (labelBuffer.empty() || teachObjectIndex < 0 || teachObjectIndex >= int(result.objects.size())) return;
	detector.getClassifier().teach(result.objects[teachObjectIndex], labelBuffer, settings);
	say("Taught '" + labelBuffer + "'");
	labelBuffer.clear();
	teachObjectIndex = -1;
	needsRun = true;
}

void ofApp::cancelTeaching() {
	typing = false;
	labelBuffer.clear();
	teachObjectIndex = -1;
}

void ofApp::resetSettings() {
	settings = DetectionSettings();
	lastSettings = settings;
	needsRun = true;
	say("Reset settings to defaults");
}

void ofApp::keyPressed(int key) {
	// While a name is being typed, every key belongs to the name.
	if (typing) {
		if (key == OF_KEY_RETURN)         finishTeaching();
		else if (key == OF_KEY_ESC)       cancelTeaching();
		else if (key == OF_KEY_BACKSPACE) { if (!labelBuffer.empty()) labelBuffer.pop_back(); }
		else if (key >= 32 && key < 127 && labelBuffer.size() < 20) labelBuffer.push_back(char(key));
		return;
	}

	switch (key) {
		case 'o': case 'O': chooseImageDialog(); break;
		case 'c': case 'C': openCamera(); break;
		case 'v': case 'V': chooseVideoDialog(); break;
		case '1': settings.mode = SegmentationMode::AutoOtsu;        break;
		case '2': settings.mode = SegmentationMode::ManualThreshold; break;
		case '3': settings.mode = SegmentationMode::Adaptive;        break;
		case '4': settings.mode = SegmentationMode::ColourPick;      break;
		case 'g': case 'G': settings.forceGrayscale = !settings.forceGrayscale; break;
		case 'i': case 'I': settings.invert = !settings.invert; break;
		case 'x': case 'X': settings.splitTouching = !settings.splitTouching; break;
		case 't': case 'T': teachMode = !teachMode; say(teachMode ? "Teach mode on" : "Teach mode off"); break;
		case OF_KEY_TAB:    view = ViewMode((int(view) + 1) % 3); break;
		case 'r': case 'R': detector.getTracker().reset(); say("Video counter reset"); break;
		case 's': case 'S': saveScreenshot(); break;
		case 'e': case 'E': exportCsv(); break;
		case 'h': case 'H': showHelp = !showHelp; break;
		case 'd': case 'D': resetSettings(); break;
		case ' ': {
			auto* v = dynamic_cast<VideoFileSource*>(source.get());
			if (v) v->togglePause();
			break;
		}
		default: break;
	}
	panel.readFrom(settings);                   // keep the sliders in step with the keys
}

// ------------------------------------------------------------------- utilities

void ofApp::say(const std::string& msg) {
	message      = msg;
	messageUntil = ofGetElapsedTimef() + 3.0f;
	ofLogNotice("KounterPlusPlus") << msg;
}

void ofApp::saveScreenshot() {
	const std::string name = "screenshot_" + ofGetTimestampString() + ".png";
	ofSaveScreen(name);                         // lands in bin/data
	say("Saved " + name);
}

void ofApp::exportCsv() {
	const std::string name = "objects_" + ofGetTimestampString() + ".csv";
	ofBuffer csv;
	csv.append("number,label,track_id,area_px,perimeter_px,width,height,centroid_x,centroid_y,circularity,aspect_ratio,solidity,hue,saturation,value\n");
	int n = 1;
	for (const auto& o : result.objects) {
		csv.append(ofToString(n++) + "," + o.label + "," + ofToString(o.trackId) + ","
				   + ofToString(o.area, 1) + "," + ofToString(o.perimeter, 1) + ","
				   + ofToString(o.bbox.width) + "," + ofToString(o.bbox.height) + ","
				   + ofToString(o.centroid.x, 1) + "," + ofToString(o.centroid.y, 1) + ","
				   + ofToString(o.circularity, 3) + "," + ofToString(o.aspectRatio, 3) + ","
				   + ofToString(o.solidity, 3) + "," + ofToString(o.meanHue, 1) + ","
				   + ofToString(o.meanSaturation, 1) + "," + ofToString(o.meanValue, 1) + "\n");
	}
	ofBufferToFile(name, csv);                  // lands in bin/data
	say("Exported " + name);
}
