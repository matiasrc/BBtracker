//
//  funcionesApp.cpp
//  sincro2Ableton
//
//  Created by Matias Romero Costas on 10/14/17.
//
//

#include "ofApp.h"

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.txt)
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

//--------------------------------------------------------------
void ofApp::drawGui(){
    //required to call this at beginning
    gui.begin();
    
    bool closeWindow = true;

    bool * p_open = &closeWindow;
    
    // Demonstrate the various window flags. Typically you would just use the default!
    static bool no_titlebar = false;
    static bool no_scrollbar = false;
    static bool no_menu = false;
    static bool no_move = false;
    static bool no_resize = false;
    static bool no_collapse = true;
    static bool no_close = true;
    static bool no_nav = false;
    static bool no_background = false;
    static bool no_bring_to_front = false;

    ImGuiWindowFlags window_flags = 0;
    if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
    if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
    if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
    if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
    if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
    if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
    if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
    if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
    if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (no_close)           p_open = NULL; // Don't pass our bool* to Begin
    
    
    // -------- MENU PRINCIPAL --------
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("| Sustracción de fondo"))
        {
            ImGui::Checkbox("ventana flotante", &show_bg_config_panel);
            ImGui::Separator();
            
            ImGui::Checkbox("Utilizar la sustraction de fondo", &useBgSubtraction); ImGui::SameLine(); HelpMarker("habilitar la substracción de fondo");
            
            ImGui::Text("Técnica: ");
            ImGui::RadioButton("absolute", &bgSubTech, 0); ImGui::SameLine();
            ImGui::RadioButton("lighter than", &bgSubTech, 1); ImGui::SameLine();
            ImGui::RadioButton("darker than", &bgSubTech, 2); ImGui::SameLine(); HelpMarker("elegirl la técnica de substracción de fondo. Determina si toma la diferencia entre la imagen de entrada y el fondo, deja pasar formas claras o formas oscuras");
            
            ImGui::Separator();
            ImGui::Checkbox("fondo adaptativo", &adaptive); ImGui::SameLine(); HelpMarker("fondo adaptativo. El fondo se actualiza según la velocidad de apatación. Si esto está desactivado el fondo es fijo");
            ImGui::SliderFloat("veloidad de adaptación", &adaptSpeed, 0.001f, 0.1f); ImGui::SameLine(); HelpMarker("velocidad a la que se actualiza el fondo. Mayor velocidad se adapta a cambios más rápidos a los cambios del fondo, a costa de una pérdida de presencia de los objetos");
            
            ImGui::Separator();
            ImGui::Text("Ajustes de imagen");
            ImGui::SliderFloat("brillo", &brightness, -1.0f, 1.0f); ImGui::SameLine(); HelpMarker("brillo de la imagen");
            ImGui::SliderFloat("contraste", &contrast, -1.0f, 1.0f); ImGui::SameLine(); HelpMarker("contraste de la imagen");
            ImGui::SliderInt("blur", &blur, 0, 30); ImGui::SameLine(); HelpMarker("desenfoque de la imagen, utilizado para eliminar ruido");
            ImGui::Checkbox("dilate", &dilate); ImGui::SameLine(); HelpMarker("dilata el límite y aumenta el tamaño del objeto en primer plano");
            ImGui::Checkbox("erode", &erode); ImGui::SameLine(); HelpMarker("erosiona el límite y reduce el tamaño del objeto en primer plano");
            
            ImGui::Separator();
            if (ImGui::Button("Capturar fondo")){
                 bLearnBackground = true;
             }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("| Blob Detection"))
        {
            ImGui::Checkbox("ventana flotante", &show_bd_config_panel);
            ImGui::Separator();
            
            ImGui::SliderFloat("umbral", &threshold, 0.0f, 255.0f); ImGui::SameLine(); HelpMarker("Umbral de brillo para detectar blobs");
            
            ImGui::SliderFloat("tamaño mínimo de objeto", &minArea, 0.01f , 50.0f); ImGui::SameLine(); HelpMarker("tamaño mínimo para detedtar un objeto, en porcentaje ");
            ImGui::SliderFloat("tamaño máximo de objeto", &maxArea, 0.2f, 100.0f); ImGui::SameLine(); HelpMarker("tamaño máximo para detedtar un objeto");
            
            ImGui::SliderInt("persistence", &persistence, 0, 100); ImGui::SameLine(); HelpMarker("cantidad de frames que un objeto puede desaparecer antes de ser olvidado por el seguidor");

            ImGui::SliderInt("distance", &distance, 0, 100); ImGui::SameLine(); HelpMarker("cantidad máxima de pixeles que se puede mover para no ser considerado obro objeto");
            
            ImGui::Separator();
            
            ImGui::Checkbox("Invertir", &invert); ImGui::SameLine(); HelpMarker("busca regiones oscuras en lugar de claras");
            ImGui::Checkbox("Buscar huecos", &findHoles); ImGui::SameLine(); HelpMarker("busca objetos (blobs) dentro de otros");
            ImGui::Checkbox("Capturar color", &trackColor); ImGui::SameLine(); HelpMarker("detección de color (capturar un color de la pantalla mientras se presiona la tecla c");
            ImGui::SameLine();
            ImGui::Checkbox("Modo HS", &trackHs); ImGui::SameLine(); HelpMarker("utilizar detección HS (con tilde) o RGB (sin tilde)");

            contourFinder.setThreshold(threshold);
            contourFinder.setMinArea(minArea / 100 * imagePixelNumber);
            contourFinder.setMaxArea(maxArea / 100 * imagePixelNumber);
            contourFinder.setInvert(invert); // find black instead of white
            contourFinder.setFindHoles(findHoles);
            // wait for half a second before forgetting something
            contourFinder.getTracker().setPersistence(persistence);
            // an object can move up to 32 pixels per frame
            contourFinder.getTracker().setMaximumDistance(distance);
            contourFinder.setTargetColor(targetColor, trackHs ? ofxCv::TRACK_COLOR_HS : ofxCv::TRACK_COLOR_RGB); // define si detecta o no un color

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("| Entrada"))
        {
            // Using the _simplified_ one-liner Combo() api here
            // See "Combo" section for examples of how to use the more complete BeginCombo()/EndCombo() api.
            //const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", //"KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
            /*
            const char* items[devicesVector.size()];
            //static int item_current = 0;
            for(int i=0; i<devicesVector.size(); i++){
                
                items[i] = devicesVector[i].c_str();
            }
            if(ImGui::Combo(" ", &deviceID, items, IM_ARRAYSIZE(items))){
                resetCameraSettings(deviceID);
                ofLogVerbose() << "--------CAMBIO DE CAMARA - DIVICE ID: " << deviceID;
            }
            */
            
            int selectedIndex = deviceID;
            if(ImGui::BeginCombo(" ", devicesVector[0].c_str())){
    
                for(int i=0; i < devicesVector.size(); ++i){
                    const bool isSelected = (selectedIndex == i);
                    if(ImGui::Selectable(devicesVector[i].c_str(), isSelected)){
                        selectedIndex = i;
                        resetCameraSettings(selectedIndex);
                    }
                    if(isSelected){
                        ImGui::SetItemDefaultFocus();
                    }
                }
            ImGui:ImGui::EndCombo();
            }
            ImGui::SameLine(); HelpMarker("Elegir el dispositivo de entrada");
            
            ImGui::Separator();
            ImGui::Checkbox("ESPEJAR HORIZONTAL", &hMirror);
            ImGui::Checkbox("ESPEJAR VERTICAL", &vMirror);
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("| OSC"))
        {
            if(ImGui::InputInt("port", &puerto)) sender.setup(host, puerto);
            ImGui::SameLine(); HelpMarker("puerto de conección");
            
            static char str1[128];
            strcpy(str1, host.c_str());
            //static char str1[128] = "127.0.0.1";
            //ImGui::InputTextWithHint("ip", "enter ip address here", str1, IM_ARRAYSIZE(str1));
            if( ImGui::InputTextWithHint("ip", "enter ip address here",str1, IM_ARRAYSIZE(str1))){
                host = ofVAArgsToString(str1);
                sender.setup(host, puerto);
                //ofLogVerbose() << "--------CAMBIO DE HOST: " << host;
            }
            ImGui::SameLine(); HelpMarker("dirección ip del receptor de mensajes");
            
            ImGui::Separator();
            
            static char blobsaddress[128];
            strcpy(blobsaddress, etiquetaMensajeBlobs.c_str());
            if( ImGui::InputTextWithHint("address1", "tipear etiqueta BLOBS",blobsaddress, IM_ARRAYSIZE(blobsaddress))){
                etiquetaMensajeBlobs = ofVAArgsToString(blobsaddress);
                //ofLogVerbose() << "--------CAMBIO DE ETIQUETA: " << movimientoaddress;
            }
            ImGui::SameLine(); HelpMarker("etiqueta (debe comenzar con /) ");
            
            ImGui::Checkbox("ENVIAR BLOBS", &enviarBlobs);
            ImGui::SameLine(); HelpMarker("habilitar / deshabilitar el envío de blobs");
            
            static char contourssaddress[128];
            strcpy(contourssaddress, etiquetaMensajeContornos.c_str());
            if( ImGui::InputTextWithHint("address2", "tipear etiqueta CONTORNOS",contourssaddress, IM_ARRAYSIZE(contourssaddress))){
                etiquetaMensajeContornos = ofVAArgsToString(contourssaddress);
                //ofLogVerbose() << "--------CAMBIO DE ETIQUETA: " << movimientoaddress;
            }
            ImGui::SameLine(); HelpMarker("etiqueta (debe comenzar con /) ");
            
            ImGui::Checkbox("ENVIAR CONTORNOS", &enviarContornos);
            ImGui::SameLine(); HelpMarker("habilitar / deshabilitar el envío de contornos");
            
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("| Ver"))
        {
            ImGui::RadioButton("original", &bgImageShow, 0);
            ImGui::RadioButton("escala de grises", &bgImageShow, 1);
            ImGui::RadioButton("fondo", &bgImageShow, 2);
            ImGui::RadioButton("bitonal", &bgImageShow, 3); ImGui::SameLine(); HelpMarker("elegirl la imagen que se muestra");
            
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("| Acerca"))
        {
            ImGui::Text("BBlobTracker");
            ImGui::Separator();
            ImGui::Text("Software experimental para captura de movimiento.");
            ImGui::Text("utilizando las técnicas de sustracción de fondo, ");
            ImGui::Text("detección de blobs y detección de color.");
            ImGui::Text("Esta aplicación está en desarrollo y no tiene soporte");
            ImGui::Text("..............");
            ImGui::Text("Desarrollado por Matías Romero Costas (Biopus)");
            ImGui::Text("www.biopus.ar");

            ImGui::EndMenu();
        }
        /*
        if (ImGui::BeginMenu("| Guardar", "cmd+s"))
        {
            saveSettings();
            ofLogVerbose() << "Configuración guardada";
            if (ImGui::MenuItem("Guardar", "cmd+s")) {
                saveSettings();
                ofLogVerbose() << "Configuración guardada";
            }
            
            ImGui::EndMenu();
        }
        */
        ImGui::EndMainMenuBar();
    }
    // Menú flotante
    
    if (show_bg_config_panel)
    {

        ImGui::SetNextWindowSize(ofVec2f(450,350), ImGuiCond_FirstUseEver);
        ImGui::Begin("Sustracción de fondo", &show_bg_config_panel);
        
        {
           ImGui::Checkbox("Utilizar la sustraction de fondo", &useBgSubtraction); ImGui::SameLine(); HelpMarker("habilitar la substracción de fondo");
           
           ImGui::Text("Técnica: ");
           ImGui::RadioButton("absolute", &bgSubTech, 0); ImGui::SameLine();
           ImGui::RadioButton("lighter than", &bgSubTech, 1); ImGui::SameLine();
           ImGui::RadioButton("darker than", &bgSubTech, 2); ImGui::SameLine(); HelpMarker("elegirl la técnica de substracción de fondo. Determina si toma la diferencia entre la imagen de entrada y el fondo, deja pasar formas claras o formas oscuras");
           
           ImGui::Separator();
           ImGui::Checkbox("fondo adaptativo", &adaptive); ImGui::SameLine(); HelpMarker("fondo adaptativo. El fondo se actualiza según la velocidad de apatación. Si esto está desactivado el fondo es fijo");
           ImGui::SliderFloat("veloidad de adaptación", &adaptSpeed, 0.001f, 0.1f); ImGui::SameLine(); HelpMarker("velocidad a la que se actualiza el fondo. Mayor velocidad se adapta a cambios más rápidos a los cambios del fondo, a costa de una pérdida de presencia de los objetos");
           
           ImGui::Separator();
           ImGui::Text("Ajustes de imagen");
           ImGui::SliderFloat("brillo", &brightness, -1.0f, 1.0f); ImGui::SameLine(); HelpMarker("brillo de la imagen");
           ImGui::SliderFloat("contraste", &contrast, -1.0f, 1.0f); ImGui::SameLine(); HelpMarker("contraste de la imagen");
           ImGui::SliderInt("blur", &blur, 0, 30); ImGui::SameLine(); HelpMarker("desenfoque de la imagen, utilizado para eliminar ruido");
           ImGui::Checkbox("dilate", &dilate); ImGui::SameLine(); HelpMarker("dilata el límite y aumenta el tamaño del objeto en primer plano");
           ImGui::Checkbox("erode", &erode); ImGui::SameLine(); HelpMarker("erosiona el límite y reduce el tamaño del objeto en primer plano");
           
           ImGui::Separator();
           if (ImGui::Button("Capturar fondo")){
                bLearnBackground = true;
            }
        }
        
        ImGui::End();
    }
    
    if (show_bd_config_panel)
    {

        ImGui::SetNextWindowSize(ofVec2f(350,400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Blob detection", &show_bd_config_panel);
        
        {
           ImGui::SliderFloat("umbral", &threshold, 0.0f, 255.0f); ImGui::SameLine(); HelpMarker("Umbral de brillo para detectar blobs");
           
           ImGui::SliderFloat("tamaño mínimo de objeto", &minArea, 0.01f , 50.0f); ImGui::SameLine(); HelpMarker("tamaño mínimo para detedtar un objeto, en porcentaje ");
           ImGui::SliderFloat("tamaño máximo de objeto", &maxArea, 0.2f, 100.0f); ImGui::SameLine(); HelpMarker("tamaño máximo para detedtar un objeto");
           
           ImGui::SliderInt("persistence", &persistence, 0, 100); ImGui::SameLine(); HelpMarker("cantidad de frames que un objeto puede desaparecer antes de ser olvidado por el seguidor");

           ImGui::SliderInt("distance", &distance, 0, 100); ImGui::SameLine(); HelpMarker("cantidad máxima de pixeles que se puede mover para no ser considerado obro objeto");
           
           ImGui::Separator();
           
           ImGui::Checkbox("Invertir", &invert); ImGui::SameLine(); HelpMarker("busca regiones oscuras en lugar de claras");
           ImGui::Checkbox("Buscar huecos", &findHoles); ImGui::SameLine(); HelpMarker("busca objetos (blobs) dentro de otros");
           ImGui::Checkbox("Capturar color", &trackColor); ImGui::SameLine(); HelpMarker("detección de color (capturar un color de la pantalla mientras se presiona la tecla c");
           ImGui::SameLine();
           ImGui::Checkbox("Modo HS", &trackHs); ImGui::SameLine(); HelpMarker("utilizar detección HS (con tilde) o RGB (sin tilde)");

           contourFinder.setThreshold(threshold);
           contourFinder.setMinArea(minArea / 100 * imagePixelNumber);
           contourFinder.setMaxArea(maxArea / 100 * imagePixelNumber);
           contourFinder.setInvert(invert); // find black instead of white
           contourFinder.setFindHoles(findHoles);
           // wait for half a second before forgetting something
           contourFinder.getTracker().setPersistence(persistence);
           // an object can move up to 32 pixels per frame
           contourFinder.getTracker().setMaximumDistance(distance);
            contourFinder.setTargetColor(targetColor, trackHs ? ofxCv::TRACK_COLOR_HS : ofxCv::TRACK_COLOR_RGB); // define si detecta o no un color
        }
        
        ImGui::End();
    }
    gui.end();
}
