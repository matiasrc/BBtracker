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
    //viewCam            = false;  // en el XML
    camWidth            = 640;
    camHeight           = 480;
    imagePixelNumber    = camWidth * camHeight;
    //deviceID            = 1;
    deviceName          = "NO DEVICE AVAILABLE";

    //hMirror             = false; // en el XML
    //vMirror             = false; // en el XML
       
    isOneDeviceAvailable =  false;
    
    setupCam(deviceID);
    
    imitate(camPixels, cam);
    
    //-------BG SUBTRACTION ------
    
    colorImg.allocate(camWidth,camHeight);
    grayImg.allocate(camWidth,camHeight);
    grayBg.allocate(camWidth,camHeight);
    bitonal.allocate(camWidth,camHeight);
    
    adaptiveBg.allocate(camWidth, camHeight, OF_IMAGE_GRAYSCALE );
    /*
    useBgSubtraction = false;
    bLearnBackground = false;
    
    brightness = 0;
    contrast = 0;
    bgSubTech = 0;
    dilate = false;
    erode = false;
    blur = 1;
    adaptive = true;
    adaptSpeed = 0.01f;
    */
    
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
    
    fullScreen = false;
    
    //----------------- OSC -------------------
    
    sender.setup(host, puerto);
}

void ofApp::update() {
	cam.update();
    
    float w = ofGetWidth();
    float h = ofGetHeight();
    
	if(cam.isFrameNew()) {
        
        copy(cam, camPixels);
        
        camPixels.mirror(vMirror, hMirror);
        
        if(useBgSubtraction){
            
            //------BG SUBTRACTION-------------
            
            colorImg.setFromPixels(camPixels);
            colorImg.updateTexture();
            
            grayImg = colorImg;
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
            contourFinder.findContours(camPixels);
        }
        
        activeBlobs = contourFinder.size();
        
        for(int i=0; i<activeBlobs; i++){
            
            int label = contourFinder.getLabel(i);
            int age = contourFinder.getTracker().getAge(label);
            //Centers
            ofVec2f centroid = toOf(contourFinder.getCentroid(i));
            ofVec2f average = toOf(contourFinder.getAverage(i));
            ofVec2f center = toOf(contourFinder.getCenter(i));
            
            // velocity
            ofVec2f velocity = toOf(contourFinder.getVelocity(i));
            
            // area and perimeter
            double area = contourFinder.getContourArea(i);
            double perimeter = contourFinder.getArcLength(i);

            // bounding rect
            cv::Rect boundingRect = contourFinder.getBoundingRect(i);

            // contour
            ofPolyline contour = toOf(contourFinder.getContour(i));

            if (enviarBlobs){
                vector<float> dataBlobs;
                
                // 2
                dataBlobs.push_back(static_cast<float>(label));         // 0
                dataBlobs.push_back(static_cast<float>(age));           // 1

                // 6
                dataBlobs.push_back(centroid.x / camWidth);             // 2
                dataBlobs.push_back(centroid.y / camHeight);            // 3
                dataBlobs.push_back(average.x / camWidth);              // 4
                dataBlobs.push_back(average.y / camHeight);             // 5
                dataBlobs.push_back(center.x / camWidth);               // 6
                dataBlobs.push_back(center.y / camHeight);              // 7

                // 2
                dataBlobs.push_back(velocity.x / camWidth);             // 8
                dataBlobs.push_back(velocity.y / camHeight);            // 9

                // 2
                dataBlobs.push_back(area / (camWidth * camHeight));     // 10
                dataBlobs.push_back(perimeter / (camWidth * camHeight));// 11

                // 4
                dataBlobs.push_back(boundingRect.x / camWidth);         // 12
                dataBlobs.push_back(boundingRect.y / camHeight);        // 13
                dataBlobs.push_back(boundingRect.width / camWidth);     // 14
                dataBlobs.push_back(boundingRect.height / camHeight);   // 15
                
                // 1
                dataBlobs.push_back(contour.getVertices().size());      // 16
                
                if(enviarContornos){
                    for(int c=0;c<contour.getVertices().size();c++){    // 17+
                        dataBlobs.push_back(contour.getVertices().at(c).x / camWidth);
                        dataBlobs.push_back(contour.getVertices().at(c).y / camHeight);
                    }
                }
                enviarOsc(etiquetaMensajeBlobs, dataBlobs);
            }
        }
        
    }
    ofSetFullscreen(fullScreen);
}

void ofApp::draw() {
    
    ofSetColor(255);
    
    float w = ofGetWidth();
    float h = ofGetHeight();
    
    if(useBgSubtraction){
        if(bgImageShow == 0){
            colorImg.draw(0, 0, w, h);
        }else if(bgImageShow == 1){
            grayImg.draw(0, 0, w, h);
        }
        else if(bgImageShow == 2){
            grayBg.draw(0, 0, w, h);
        }
        else{
            bitonal.draw(0, 0, w, h);
        }
    }else{
        camPixels.draw(0, 0, w, h);
    }
    
    ofSetColor(0, 255, 0);
    //contourFinder.draw();
    
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
    
    drawGui();
    
    if(trackColor && !useBgSubtraction){
        ofTranslate(ofGetWidth() - 70, ofGetHeight() - 70);
        ofFill();
        ofSetColor(0);
        ofDrawRectangle(-3, -3, 64+6, 64+6);
        ofSetColor(targetColor);
        ofDrawRectangle(0, 0, 64, 64);
    }
    
    ofSetWindowTitle("FPS: " + ofToString(ofGetFrameRate()));
}

void ofApp::setupCam(int devID){
    
    wdevices = cam.listDevices();
    for(int i=0;i<static_cast<int>(wdevices.size());i++){
        if(wdevices[i].bAvailable){
            isOneDeviceAvailable = true;
            devicesVector.push_back(wdevices[i].deviceName);
            devicesID.push_back(i);

            for(size_t f=0;f<wdevices[i].formats.size();f++){
                ofLog(OF_LOG_NOTICE,"Capture Device format vailable: %ix%i",wdevices[i].formats.at(f).width,wdevices[i].formats.at(f).height);
            }
        }
    }
    
    cam.setDeviceID(devID);
    cam.setup(camWidth, camHeight);
}
void ofApp::resetCameraSettings(int devID){
    if(devID!=deviceID){
        ofLog(OF_LOG_NOTICE,"Changing Device to: %s",devicesVector[devID].c_str());

        deviceID = devID;
        deviceName = devicesVector[deviceID];
    }
    
    if(cam.isInitialized()){
        cam.close();
        //cam = new ofVideoGrabber();
        cam.setDeviceID(deviceID);
        cam.setup(camWidth, camHeight);
    }
    
}

//--------------------------------------------------------------
void ofApp::enviarOsc(string etiqueta, float valor){
    ofxOscMessage m;
    m.setAddress(etiqueta);
    m.addFloatArg(valor);
    sender.sendMessage(m, false);
}
//--------------------------------------------------------------
void ofApp::enviarOsc(string etiqueta, vector<float> valores){
    ofxOscMessage m;
    m.setAddress(etiqueta);
    for( int i=0; i<valores.size(); i++){
        m.addFloatArg(valores[i]);
    }
    sender.sendMessage(m, false);
    
    
    /*
     [0] --> blob ID
     [1] --> blob age (milliseconds)
     [2] --> blob centroid X
     [3] --> blob centroid Y
     [4] --> blob average X
     [5] --> blob average Y
     [6] --> blob center X
     [7] --> blob center Y
     [8] --> blob velocity X
     [9] --> blob velocity Y
     [10] --> blob area
     [11] --> blob perimeter
     [12] --> blob bounding rect X
     [13] --> blob bounding rect Y
     [14] --> blob bounding rect Width
     [15] --> blob bounding rect Height
     [16] --> Number of Contours vertices
     if(enviarContornos){
        [17+] --> Contours vertices
     }
    */
    
}
//--------------------------------------------------------------
void ofApp::loadSettings(){
    //-----------
    //the string is printed at the top of the app
    //to give the user some feedback
    xmlMessage = "loading mySettings.xml";

    //we load our settings file
    //if it doesn't exist we can still make one
    //by hitting the 's' key
    if( XML.loadFile("mySettings.xml") ){
        xmlMessage = "mySettings.xml loaded!";
    }else{
        xmlMessage = "unable to load mySettings.xml check data/ folder";
    }

    //read the colors from XML
    //if the settings file doesn't exist we assigns default values (170, 190, 240)
    //red        = XML.getValue("BACKGROUND:COLOR:RED", 170);
    
    //---------------- OSC --------------------
    puerto = XML.getValue("OSC:PUERTO", 3333);
    host = XML.getValue("OSC:HOST", "127.0.0.1");
    etiquetaMensajeBlobs = XML.getValue("OSC:ETIQUETA:BLOBS", "/bblobtracke/blobs");
    enviarBlobs = XML.getValue("OSC:ENVIARBLOBS", true);
    enviarContornos = XML.getValue("OSC:ENVIARCONTORNOS", false);
    
    //---------------- CAM --------------------
    deviceID = XML.getValue("CAM:DEVICEID", 0);
    hMirror = XML.getValue("CAM:HMIRROR", false);
    vMirror = XML.getValue("CAM:VMIRROR", false);
    
    //---------------- BG SUBTRACTION --------------------
    useBgSubtraction = XML.getValue("BGSUBTRACTION:USEBGSUBTRACTION", false);
    bgSubTech = XML.getValue("BGSUBTRACTION:BGSUBTECHNIQUE", 0);
    brightness = XML.getValue("BGSUBTRACTION:BRIGHTNESS", 0);
    contrast = XML.getValue("BGSUBTRACTION:CONTRAST", 0);
    blur = XML.getValue("BGSUBTRACTION:BLUR", 1);
    dilate = XML.getValue("BGSUBTRACTION:DILATE", false);
    erode = XML.getValue("BGSUBTRACTION:ERODE", false);
    adaptive = XML.getValue("BGSUBTRACTION:ADAPTATIVE", false);
    adaptSpeed = XML.getValue("BGSUBTRACTION:ADAPTSPEED", 0.01);
    
    //---------------- CONTOUR FINDER --------------------
    threshold = XML.getValue("CV:THRESHOLD", 127);
    minArea = XML.getValue("CV:MINAREA", 0.0);
    maxArea = XML.getValue("CV:MAXAREA", 1.0);
    persistence = XML.getValue("CV:PERSISTENCE", 15);
    distance = XML.getValue("CV:DISTANCE", 30);
    invert = XML.getValue("CV:INVERT", false);
    findHoles = XML.getValue("CV:FINDHOLES", false);
    trackColor = XML.getValue("CV:TRACKCOLOR", false);
    trackHs = XML.getValue("CV:TRACKHS", false);

    
    ofLog(OF_LOG_NOTICE,xmlMessage);
}
//--------------------------------------------------------------
void ofApp::saveSettings(){
    //XML.setValue("BACKGROUND:COLOR:RED", red);
    XML.clear();

    //---------------- OSC --------------------
    XML.setValue("OSC:PUERTO", puerto);
    XML.setValue("OSC:HOST", host);
    XML.setValue("OSC:ETIQUETA:BLOBS", etiquetaMensajeBlobs);
    XML.setValue("OSC:ENVIARBLOBS", enviarBlobs);
    XML.setValue("OSC:ETIQUETA:CONTORNOS", etiquetaMensajeBlobs);
    XML.setValue("OSC:ENVIARCONTORNOS", enviarContornos);
    
    //---------------- CAM --------------------
    XML.setValue("CAM:DEVICEID", deviceID);
    XML.setValue("CAM:HMIRROR", hMirror);
    XML.setValue("CAM:VMIRROR", vMirror);
    
    //---------------- BG SUBTRACTION --------------------
    XML.setValue("BGSUBTRACTION:USEBGSUBTRACTION", useBgSubtraction);
    XML.setValue("BGSUBTRACTION:BGSUBTECHNIQUE", bgSubTech);
    XML.setValue("BGSUBTRACTION:BRIGHTNESS", brightness);
    XML.setValue("BGSUBTRACTION:CONTRAST", contrast);
    XML.setValue("BGSUBTRACTION:BLUR", blur);
    XML.setValue("BGSUBTRACTION:DILATE", dilate);
    XML.setValue("BGSUBTRACTION:ERODE", erode);
    XML.setValue("BGSUBTRACTION:ADAPTATIVE", adaptive);
    XML.setValue("BGSUBTRACTION:ADAPTSPEED", adaptSpeed);
    
    //---------------- CONTOUR FINDER --------------------
    XML.setValue("CV:THRESHOLD", threshold);
    XML.setValue("CV:MINAREA", minArea);
    XML.setValue("CV:MAXAREA", maxArea);
    XML.setValue("CV:PERSISTENCE", persistence);
    XML.setValue("CV:DISTANCE", distance);
    XML.setValue("CV:INVERT", invert);
    XML.setValue("CV:FINDHOLES", findHoles);
    XML.setValue("CV:TRACKCOLOR", trackColor);
    XML.setValue("CV:TRACKHS", trackHs);

        
    XML.saveFile("mySettings.xml");
    xmlMessage ="settings saved to xml!";
    ofLog(OF_LOG_NOTICE,xmlMessage);
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
    }else if(e.key == ' '){
        bLearnBackground = true;
    }else if(e.key == 'f'){
        fullScreen = !fullScreen;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(ofKeyEventArgs& e){
    if(e.key == 'c'){
        captureColor = false;
    }
}

void ofApp::mousePressed(int x, int y, int button) {
    
    float nx = ofMap(x, 0, ofGetWidth(), 0, camWidth);
    float ny = ofMap(y, 0, ofGetHeight(), 0, camHeight);
    
    if(trackColor && captureColor && !useBgSubtraction){
        targetColor = camPixels.getColor(nx, ny);
    }
}
