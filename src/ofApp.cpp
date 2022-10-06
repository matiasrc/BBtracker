#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

void ofApp::setup() {
    ofSetFrameRate(60);
    //ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(true);
    
    //----------------- XML -------------------
    loadSettings();
    
    //----------------- CAM -------------------
    camWidth            = 640;
    camHeight           = 480;
    imagePixelNumber    = camWidth * camHeight;
    //deviceID          = 1;
    deviceName          = "NO DEVICE AVAILABLE";

    isOneDeviceAvailable =  false;
    
    setupCam(deviceID);
    
    //----------------- WARP -------------------
    warpON =  false;
    cualPunto = 0;
   
    moverPunto = false;
    
    mirroredImg.allocate(camWidth,camHeight);
    warpedImg.allocate(camWidth,camHeight);
    

    //-------BG SUBTRACTION ------
    
    grayImg.allocate(camWidth,camHeight);
    grayBg.allocate(camWidth,camHeight);
    bitonal.allocate(camWidth,camHeight);
    adaptiveBg.allocate(camWidth, camHeight, OF_IMAGE_GRAYSCALE );
    
    //-----------COUNTOUR FINDER ---------
    captureColor = false;
    contourFinder.setThreshold(threshold);
    contourFinder.setMinArea(minArea / 100 * imagePixelNumber);
    contourFinder.setMaxArea(maxArea / 100 * imagePixelNumber);
    contourFinder.setInvert(invert); // find black instead of white
    contourFinder.setFindHoles(findHoles);
    // wait for half a second before forgetting something
    contourFinder.getTracker().setPersistence(persistence);
    // an object can move up to 32 pixels per frame
    contourFinder.getTracker().setMaximumDistance(distance);
    
    activeBlobs = 0;
    //----------------- GUI -------------------
    //required call
    gui.setup();
    
    ImGui::GetIO().MouseDrawCursor = false;
    
    show_bg_config_panel = false;
    show_bd_config_panel = false;
            
    //----------------- OSC -------------------
    
    sender.setup(host, puerto);
}

void ofApp::update() {
	cam.update();
    
    float w = ofGetWidth();
    float h = ofGetHeight();
    
	if(cam.isFrameNew()) {
                
        mirroredImg.setFromPixels(cam.getPixels());
        mirroredImg.mirror(vMirror, hMirror);
        
        warpedImg = mirroredImg;
        warpedImg.warpPerspective(warp[0], warp[1], warp[2], warp[3]);
                        
        if(useBgSubtraction){
        
            //------BG SUBTRACTION-------------

            grayImg = warpedImg;
            grayImg.updateTexture();
            grayImg.brightnessContrast(brightness,contrast);

            if(bgSubTech == 0){ // B&W ABS
                bitonal.absDiff(grayBg, grayImg);
            }else if(bgSubTech == 1){ // LIGHTER THAN
                bitonal = grayImg;
                bitonal -= grayBg;
            }else if(bgSubTech == 2){ // DARKER THAN
                bitonal = grayBg;
                bitonal -= grayImg;
            }

            bitonal.threshold(threshold);
            
            if(erode){
                bitonal.erode();
            }
            if(dilate){
                bitonal.dilate();
            }
            if( blur %2 == 0){
                bitonal.blur(blur+1);
            }else{
                bitonal.blur(blur);
            }
            bitonal.updateTexture();
            
            //////////////////////////////////////////////
            // background learning

            // static
            if(bLearnBackground == true){
                bLearnBackground = false;
                grayBg = grayImg;
                grayBg.updateTexture();
                ofLogNotice()<< "captura fondo una vez";
            }

            // adaptive
            if(adaptive){
                
                lerp(grayImg, adaptiveBg, adaptiveBg, adaptSpeed);
                
                //Mat temp = toCv(grayBg);
                //temp = toCv(grayBg)*(1.0f-adaptSpeed) + toCv(grayImg)*adaptSpeed;
                //toOf(temp, grayBg.getPixels());

                grayBg = adaptiveBg;
                grayBg.updateTexture();
                
                //finalBg = grayBg;
                //finalBg.updateTexture();
            }
            //------- CONTOUR FINDER ---------
            contourFinder.setUseTargetColor(false);
            contourFinder.findContours(bitonal);
        }else{
            if(trackColor){
                contourFinder.setTargetColor(targetColor, trackHs ? TRACK_COLOR_HS : TRACK_COLOR_RGB);
            }
            //------- CONTOUR FINDER ---------
            contourFinder.findContours(warpedImg);
        }
        
        //------- OSC ---------
        updateOsc();
    }
    
    warpingReset();
}

void ofApp::draw() {
    
    ofSetColor(255);
    
    float w = ofGetWidth();
    float h = ofGetHeight();
    
    if(imageView == 0){
        mirroredImg.draw(0, 0, w, h);
    }
    else if(imageView == 1){
        warpedImg.draw(0, 0, w, h);
    }
    else if(imageView == 2){
        grayImg.draw(0, 0, w, h);
    }
    else if(imageView == 3){
        grayBg.draw(0, 0, w, h);
    }
    else{
        bitonal.draw(0, 0, w, h);
    }
    
    if(warpON){
        ofPushStyle();
        ofFill();
        ofPolyline pl;
        
        float cornerSize = 15;
        
        for(int i=0; i<4; i++){
            float x = warp[i].x / camWidth * w;
            float y = (warp[i].y / camHeight * h);
        
            pl.addVertex(x, y);
            
            corner[i].setFromCenter(x, y, cornerSize, cornerSize);
            
            ofFill();
            //ofDrawCircle(x, y, 5);
            if(i == cualPunto){
                ofSetColor(255, 0, 0);
            }else{
                ofSetColor(0, 255, 255);
            }
            ofDrawRectangle(corner[i]);
        }
        ofSetColor(0, 255, 255);
        pl.close();
        pl.draw();
        ofPopStyle();
    }else{
        ofSetColor(0, 255, 0);
        
        ofNoFill();

        for(int i = 0; i < activeBlobs; i++) {
         
            // some different styles of contour centers
            ofVec2f centroid = toOf(contourFinder.getCentroid(i));
            ofVec2f average = toOf(contourFinder.getAverage(i));
            ofVec2f center = toOf(contourFinder.getCenter(i));
            ofSetColor(cyanPrint);
            ofDrawCircle(centroid.x / camWidth * w, centroid.y / camHeight * h, 4);
            ofSetColor(magentaPrint);
            ofDrawCircle(average.x / camWidth * w, average.y / camHeight * h, 4);
            ofSetColor(yellowPrint);
            ofDrawCircle(center.x / camWidth * w, center.y / camHeight * h, 4);

            // you can also get the area and perimeter using ofPolyline:
            // ofPolyline::getArea() and ofPolyline::getPerimeter()
            double area = contourFinder.getContourArea(i);
            double perimeter = contourFinder.getArcLength(i);
            
            
            ofSetColor(0, 0, 255);
            cv::Rect r = contourFinder.getBoundingRect(i);
            ofRectangle br = toOf(r);
            ofDrawRectangle(br.x / camWidth * w, br.y / camHeight * h, br.width / camWidth * w, br.height / camHeight * h);
            
            ofSetColor(0, 255, 0);
            for(std::size_t j = 0; j < contourFinder.getContour(i).size()-1; j++) {
                cv::Point p1 = contourFinder.getContour(i).at(j);
                cv::Point p2 = contourFinder.getContour(i).at(j+1);
                ofDrawLine(p1.x / camWidth * w, p1.y / camHeight * h,
                           p2.x / camWidth * w, p2.y / camHeight * h);
            }
            
            ofSetColor(yellowPrint);
            //ofPoint center = toOf(contourFinder.getCenter(i));
            ofPushMatrix();
            ofTranslate(center.x / camWidth * w, center.y / camHeight * h);
            int label = contourFinder.getLabel(i);
            string msg1 = "ID: " + ofToString(label);
            string msg2 = "AGE: " + ofToString(contourFinder.getTracker().getAge(label));
            ofDrawBitmapString(msg1, 8, 0);
            ofDrawBitmapString(msg2, 8, 15);
            
            
            ofSetColor(255, 0, 0);
            ofVec2f velocity = toOf(contourFinder.getVelocity(i));
            ofScale(5, 5);
            ofSetLineWidth(1.5);
            ofDrawLine(0, 0, velocity.x / camWidth * w, velocity.y / camHeight * h);
            ofPopMatrix();

            
            ofSetColor(255);
            if(contourFinder.getHole(i)) {
                //ofDrawBitmapStringHighlight("hole", center.x, center.y);
            }
            ofSetColor(255);
            ofDrawBitmapStringHighlight("Objetos (Blobs) encontrados: " + ofToString(activeBlobs), 5, ofGetHeight() - 5);
        }
        
        if(trackColor && !useBgSubtraction){
            ofTranslate(ofGetWidth() - 70, ofGetHeight() - 70);
            ofFill();
            ofSetColor(0);
            ofDrawRectangle(-3, -3, 64+6, 64+6);
            ofSetColor(targetColor);
            ofDrawRectangle(0, 0, 64, 64);
        }
    }
    drawGui();
    ofSetWindowTitle("FPS: " + ofToString(ofGetFrameRate()));
}

//--------------------------------------------------------------
void ofApp::keyPressed(ofKeyEventArgs& e){
    
    #if __APPLE__
    if(e.key == 's' && e.hasModifier(OF_KEY_COMMAND)){
        saveSettings();
    }
    #else
    if(e.key == 19 ){
        saveSettings();
    }
    
    #endif
    else if(e.key == 'c'){
        captureColor = true;
    }
    else if(e.key == ' '){
        bLearnBackground = true;
    }
    
    else if(e.key == '1'){
        cualPunto = 0;
    }
    else if(e.key == '2'){
        cualPunto = 1;
    }
    else if(e.key == '3'){
        cualPunto = 2;
    }
    else if(e.key == '4'){
        cualPunto = 3;
    }
    else if(e.key == 'w'){
        warpON = !warpON;
    }
    else if(e.key == OF_KEY_LEFT && warpON){
        warp[cualPunto].x -= paso;
        warp[cualPunto].x = ofClamp(warp[cualPunto].x, 0, camWidth);
    }
    else if(e.key == OF_KEY_RIGHT && warpON){
        warp[cualPunto].x += paso;
        warp[cualPunto].x = ofClamp(warp[cualPunto].x, 0, camWidth);
    }
    else if(e.key == OF_KEY_UP && warpON){
        warp[cualPunto].y -= paso;
        warp[cualPunto].y = ofClamp(warp[cualPunto].y, 0, camHeight);
    }
    else if(e.key == OF_KEY_DOWN && warpON){
        warp[cualPunto].y += paso;
        warp[cualPunto].y = ofClamp(warp[cualPunto].y, 0, camHeight);
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(ofKeyEventArgs& e){
    if(e.key == 'c'){
        captureColor = false;
    }
}
//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    float nx = ofMap(x, 0, ofGetWidth(), 0, camWidth);
    float ny = ofMap(y, 0, ofGetHeight(), 0, camHeight);
    
    if(trackColor && captureColor && !useBgSubtraction){
        targetColor = warpedImg.getPixels().getColor(nx, ny);
    }
    if(warpON){
        for(int i=0; i<4; i++){
            if(corner[i].inside(x, y)){
                cualPunto = i;
                moverPunto = true;
                break;
            }
        }
    }
}
//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    if(moverPunto){
        if(x >= 0 && x<= ofGetWidth() && y>=0 && y <=ofGetHeight()){
            warp[cualPunto].x = ofMap(x, 0, ofGetWidth(), 0, camWidth);
            warp[cualPunto].y = ofMap(y, 0, ofGetHeight(), 0, camHeight);
        }
    }
}
//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
    moverPunto = false;
}

