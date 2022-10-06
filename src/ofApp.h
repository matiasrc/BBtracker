#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"
#include "ofxImGui.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void keyPressed(ofKeyEventArgs& e);
    void keyReleased(ofKeyEventArgs& e);
    void setupCam(int devID);
    void resetCameraSettings(int devID);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseDragged(int x, int y, int button);

    //----------------- VIEW -------------------
    int                     imageView;
	
    //----------------- CAM -------------------
    ofVideoGrabber          cam;
    ofxCvColorImage         mirroredImg;
    ofxCvColorImage         warpedImg;
    vector<ofVideoDevice>   wdevices;
    vector<string>          devicesVector;
    vector<int>             devicesID;
    float                   camWidth, camHeight;
    int                     imagePixelNumber;
    string                  deviceName;
    int                     deviceID;
    bool                    needReset;
    bool                    isOneDeviceAvailable;
    bool                    hMirror, vMirror;
        
    //----------------- WARP -------------------
    bool                    warpON;
    ofPoint                 warp[4];
    ofRectangle             corner[4];
    ofPoint                 A, B, C, D;
    int                     cualPunto;
    bool                    moverPunto;
    int                     paso;
    bool                    resetWarping;
    void                    warpingReset();
    
    //----------------- BG SUBTRACTION -------------------
    ofxCvGrayscaleImage     grayImg;
    ofxCvGrayscaleImage     grayBg;
    ofxCvGrayscaleImage     bitonal;
    
    ofImage                 adaptiveBg;
    
    bool                    useBgSubtraction;
    int                     bgSubTech;
    
    float                   brightness;
    float                   contrast;
    bool                    dilate;
    bool                    erode;
    int                     blur;
    
    bool                    adaptive;
    float                   adaptSpeed;
    
    bool                    bLearnBackground;
    
    //---------------- CONTOUR FINDER --------------------
    ofxCv::ContourFinder    contourFinder;
    
    bool                    trackColor;
    bool                    trackHs;
    bool                    captureColor;
    ofColor                 targetColor;
    
    float                   threshold;
    bool                    invert;
    bool                    findHoles;
    
    float                   minArea;
    float                   maxArea;
    int                     persistence;
    int                     distance;
    
    // some different styles of contour centers
    int                     activeBlobs;

    //----------------- GUI -------------------
    void                    drawGui();
    ofxImGui::Gui           gui;

    bool                    show_bg_config_panel;
    bool                    show_bd_config_panel;
            
    //----------------- OSC -------------------
    ofxOscSender            sender;
    int                     puerto;
    string                  host;
    string                  etiquetaMensajeBlobs;
    bool                    enviarBlobs;
    bool                    enviarContornos;
    
    void                    enviarOsc(string etiqueda, float valor);
    void                    enviarOsc(string etiqueda, vector<float> valores);
    void                    updateOsc();
    
    //----------------- XML -------------------
    ofxXmlSettings          XML;
    void                    saveSettings();
    void                    loadSettings();
    string                  xmlMessage;    
};
