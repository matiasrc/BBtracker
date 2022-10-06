
#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::warpingReset(){
    
    if(resetWarping){
        A = ofPoint(0, 0);
        B = ofPoint(camWidth, 0);
        C = ofPoint(camWidth, camHeight);
        D = ofPoint(0, camHeight);
        
        warp[0] = A;
        warp[1] = B;
        warp[2] = C;
        warp[3] = D;
        resetWarping = false;
    }
}
