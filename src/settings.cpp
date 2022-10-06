

#include "ofApp.h"

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
    
    //---------------- VIEW --------------------
    imageView = XML.getValue("VIEW:IMAGEVIEW", 0);
    
    //---------------- CAM --------------------
    deviceID = XML.getValue("CAM:DEVICEID", 0);
    hMirror = XML.getValue("CAM:HMIRROR", false);
    vMirror = XML.getValue("CAM:VMIRROR", false);
    
    paso = XML.getValue("CAM:WARPING:PASO", 5);
    
    warp[0].x = XML.getValue("CAM:WARPING:AX", 0);
    warp[0].y = XML.getValue("CAM:WARPING:Ay", 0);
    warp[1].x = XML.getValue("CAM:WARPING:BX", camWidth);
    warp[1].y = XML.getValue("CAM:WARPING:BY", 0);
    warp[2].x = XML.getValue("CAM:WARPING:CX", camWidth);
    warp[2].y = XML.getValue("CAM:WARPING:CY", camHeight);
    warp[3].x = XML.getValue("CAM:WARPING:DX", 0);
    warp[3].y = XML.getValue("CAM:WARPING:DY", camHeight);
    
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
    
    //---------------- VIEW --------------------
    XML.setValue("VIEW:IMAGEVIEW", imageView);
    
    //---------------- CAM --------------------
    XML.setValue("CAM:DEVICEID", deviceID);
    XML.setValue("CAM:HMIRROR", hMirror);
    XML.setValue("CAM:VMIRROR", vMirror);
    XML.setValue("CAM:WARPING:PASO", paso);
    XML.setValue("CAM:WARPING:AX", warp[0].x);
    XML.setValue("CAM:WARPING:Ay", warp[0].y);
    XML.setValue("CAM:WARPING:BX", warp[1].x);
    XML.setValue("CAM:WARPING:BY", warp[1].y);
    XML.setValue("CAM:WARPING:CX", warp[2].x);
    XML.setValue("CAM:WARPING:CY", warp[2].y);
    XML.setValue("CAM:WARPING:DX", warp[3].x);
    XML.setValue("CAM:WARPING:DY", warp[3].y);
    
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
