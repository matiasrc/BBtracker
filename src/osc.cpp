
#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::updateOsc(){
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
