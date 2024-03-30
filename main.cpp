#define  _CRT_SECURE_NO_WARNINGS 1
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <memory>

#include "uevr/Plugin.hpp"

#define KEYDOWN false
#define KEYUP true
#define MAX_STICK 32767
#define MAX_THRESHOLD 24000

typedef struct _HAPTIC_TIMER_STRUCT
{
    const UEVR_VRData* vr;
    int MillisecondsDelay;
}   HAPTIC_TIMER_STRUCT;

void DebugPrint(char* Format, ...);
using namespace uevr;

#define PLUGIN_LOG_ONCE(...) \
    static bool _logged_ = false; \
    if (!_logged_) { \
        _logged_ = true; \
        API::get()->log_info(__VA_ARGS__); \
    }
/*
OpenXR Action Paths 
    static const inline std::string s_action_pose = "/actions/default/in/Pose";
    static const inline std::string s_action_grip_pose = "/actions/default/in/GripPose";
    static const inline std::string s_action_trigger = "/actions/default/in/Trigger";
    static const inline std::string s_action_grip = "/actions/default/in/Grip";
    static const inline std::string s_action_joystick = "/actions/default/in/Joystick";
    static const inline std::string s_action_joystick_click = "/actions/default/in/JoystickClick";

    static const inline std::string s_action_a_button_left = "/actions/default/in/AButtonLeft";
    static const inline std::string s_action_b_button_left = "/actions/default/in/BButtonLeft";
    static const inline std::string s_action_a_button_touch_left = "/actions/default/in/AButtonTouchLeft";
    static const inline std::string s_action_b_button_touch_left = "/actions/default/in/BButtonTouchLeft";

    static const inline std::string s_action_a_button_right = "/actions/default/in/AButtonRight";
    static const inline std::string s_action_b_button_right = "/actions/default/in/BButtonRight";
    static const inline std::string s_action_a_button_touch_right = "/actions/default/in/AButtonTouchRight";
    static const inline std::string s_action_b_button_touch_right = "/actions/default/in/BButtonTouchRight";

    static const inline std::string s_action_dpad_up = "/actions/default/in/DPad_Up";
    static const inline std::string s_action_dpad_right = "/actions/default/in/DPad_Right";
    static const inline std::string s_action_dpad_down = "/actions/default/in/DPad_Down";
    static const inline std::string s_action_dpad_left = "/actions/default/in/DPad_Left";
    static const inline std::string s_action_system_button = "/actions/default/in/SystemButton";
    static const inline std::string s_action_thumbrest_touch_left = "/actions/default/in/ThumbrestTouchLeft";
    static const inline std::string s_action_thumbrest_touch_right = "/actions/default/in/ThumbrestTouchRight";

*/


class ViveHaptic : public uevr::Plugin {
public:
    HAPTIC_TIMER_STRUCT m_Timer;
    const UEVR_PluginInitializeParam* m_Param;
    const UEVR_VRData* m_VR;
	
    bool m_Zooming;
	bool m_OpenXr;
	bool m_OpenVr;
    bool m_IndexIndicatorActive;
    ViveHaptic() = default;
    
    void on_dllmain(HANDLE handle) override {
    }

    void on_initialize() override {
      OutputDebugString("Initializing Vive-Haptic\n");
      m_IndexIndicatorActive = false;
         
      // This shows how to get to the API functions.
      m_Param = API::get()->param();
      m_VR = m_Param->vr;

      ZeroMemory(&m_Timer, sizeof(HAPTIC_TIMER_STRUCT));
      m_Timer.vr = m_VR;
      m_Timer.MillisecondsDelay = 600;
      
      m_OpenXr = m_VR->is_openxr();
      m_OpenVr = m_VR->is_openvr();
    }

    void on_pre_engine_tick(UEVR_UGameEngineHandle engine, float delta) override {
        PLUGIN_LOG_ONCE("Pre Engine Tick: %f", delta);
    }

    void on_post_engine_tick(UEVR_UGameEngineHandle engine, float delta) override {
        PLUGIN_LOG_ONCE("Post Engine Tick: %f", delta);
    }

    void on_pre_slate_draw_window(UEVR_FSlateRHIRendererHandle renderer, UEVR_FViewportInfoHandle viewport_info) override {
        PLUGIN_LOG_ONCE("Pre Slate Draw Window");
    }

    void on_post_slate_draw_window(UEVR_FSlateRHIRendererHandle renderer, UEVR_FViewportInfoHandle viewport_info) override {
        PLUGIN_LOG_ONCE("Post Slate Draw Window");
    }
	
    void send_key(WORD key, bool key_up) {
        INPUT input;
        ZeroMemory(&input, sizeof(INPUT));
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = key;
        if(key_up) input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }

    //*******************************************************************************************
    // This is the controller input routine. Everything happens here.
    //*******************************************************************************************
    void on_xinput_get_state(uint32_t* retval, uint32_t user_index, XINPUT_STATE* state) {
		
		if(state == NULL) return;
        if(!m_VR->is_using_controllers()) return; // If not using controllers, none of this applies.
        
        UEVR_InputSourceHandle LeftController = m_VR->get_left_joystick_source();
        UEVR_InputSourceHandle RightController = m_VR->get_right_joystick_source();
        int TempStick = 0;
        static bool F1Down      = false;
        static bool F2Down      = false;
        static bool F3Down      = false;
        static bool F4Down      = false;
        static bool InsertDown  = false;
        static bool LTDown      = false;
        static bool RTDown      = false;
        static bool SwapLtRb    = false;
        static bool RTHaptic    = false;
        
        bool LeftShifting = false;

		if (m_OpenXr == true)
		{
            UEVR_ActionHandle GripButton    = m_VR->get_action_handle("/actions/default/in/Grip");
            UEVR_ActionHandle SystemButton  = m_VR->get_action_handle("/actions/default/in/SystemButton");
            UEVR_ActionHandle LeftShift     = m_VR->get_action_handle("/actions/default/in/SystemButton");
            UEVR_ActionHandle RightGrip     = m_VR->get_action_handle("/actions/default/in/BButtonTouchRight");
            UEVR_ActionHandle RJoystick     = m_VR->get_action_handle("/actions/default/in/JoystickClick");

#if 0
            // First, we will try to see if we are using a gamepad. If start or select is active and the 
            // openxr read for this is not, we assume gamepad mode and return.
            if(state->Gamepad.wButtons & (XINPUT_GAMEPAD_START | XINPUT_GAMEPAD_BACK))
            {
                // This catches the case where we just set the controllers down and picked up the gamepad. The controllers are still
                // active but we're not using them so we want to return and not do anything.
                if (!m_VR->is_action_active(BTouchLeft, LeftController) && !m_VR->is_action_active(ATouchLeft, LeftController))
                {
                    return;
                }
               
            }
#endif
           
            // This is only here because I have vive controllers. The index controllers right grip is active just holding
            // the controllers. So this whole plugin mechanism will be idle until the right grip is detected once.
            // In the vive version of this plugin, it will be the opposite.
            if(m_IndexIndicatorActive == false && m_VR->is_action_active(RightGrip, RightController)) {
                API::get()->log_info("Vive-Haptic: detected grip down, assuming controller is an index and disabling plugin.");
                m_IndexIndicatorActive = true;
            }
            
            if(m_IndexIndicatorActive == true) return;
            
            // clear all key down events from last pass
            if(InsertDown == true && !(state->Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
                send_key(VK_INSERT, KEYUP);
                InsertDown = false;
            }

            if(F2Down == true && !(state->Gamepad.wButtons & XINPUT_GAMEPAD_X)) {
                send_key(VK_F2, KEYUP);
                F2Down = false;
            }
            
            if(F3Down == true && !(state->Gamepad.wButtons & XINPUT_GAMEPAD_B)) {
                send_key(VK_F3, KEYUP);
                F3Down = false;
            }

            if(F4Down == true && !(state->Gamepad.wButtons & XINPUT_GAMEPAD_Y)) {
                send_key(VK_F4, KEYUP);
                F4Down = false;
            }

            if(LTDown == true && state->Gamepad.bLeftTrigger < 200) {
                LTDown = false;
            }
            
            if(RTDown == true && state->Gamepad.bRightTrigger < 200) {
                RTDown = false;
            }

            // The dpad and the left stick share the left trackpad. The dpad is active if the pad is clicked.
            // So we will check for dpad inputs and zero out the thumbsticks, if detected.
            if(state->Gamepad.wButtons & (XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT | XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_UP)) {
                state->Gamepad.sThumbLX = 0;
                state->Gamepad.sThumbLY = 0;
            }
            
            // The the X, Y, A, B buttons are on the right disk in N, S, W, E positions. 
            // Like the dpad, if any of these are active, zero the right axis.
            if(state->Gamepad.wButtons & (XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_B | XINPUT_GAMEPAD_X | XINPUT_GAMEPAD_Y)) {
                state->Gamepad.sThumbRX = 0;
                state->Gamepad.sThumbRY = 0;
                state->Gamepad.wButtons &= ~(XINPUT_GAMEPAD_RIGHT_THUMB);
            }
            int TempStick1=state->Gamepad.sThumbRX;
            // This attempts to prevent the edges where you click A, X, Y, B from registering stick actions.
            if(state->Gamepad.sThumbRY > MAX_THRESHOLD || state->Gamepad.sThumbRY < -MAX_THRESHOLD) state->Gamepad.sThumbRY = 0;
            if(state->Gamepad.sThumbRX > MAX_THRESHOLD || state->Gamepad.sThumbRX < -MAX_THRESHOLD) state->Gamepad.sThumbRX = 0;
            
            TempStick = state->Gamepad.sThumbRY * MAX_STICK / MAX_THRESHOLD;
            if(TempStick > MAX_STICK) TempStick = MAX_STICK;
            else if(TempStick < -MAX_STICK) TempStick = -MAX_STICK;
            state->Gamepad.sThumbRY = (SHORT)(TempStick);
            
            TempStick = state->Gamepad.sThumbRX * MAX_STICK / MAX_THRESHOLD;
            if(TempStick > MAX_STICK) TempStick = MAX_STICK;
            else if(TempStick < -MAX_STICK) TempStick = -MAX_STICK;
            state->Gamepad.sThumbRX = (SHORT)(TempStick);
            if(state->Gamepad.sThumbRX != 0) API::get()->log_info("Tempstick: %d-->%d", TempStick1, state->Gamepad.sThumbRX);
            
            // Check for left shifting. This is when left trackpad touched but not start or back.
            if(m_VR->is_action_active(LeftShift, LeftController)) {
                LeftShifting = true;
                
                if(state->Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) {
                    state->Gamepad.wButtons &= ~(XINPUT_GAMEPAD_LEFT_SHOULDER);
                    state->Gamepad.wButtons |= (XINPUT_GAMEPAD_LEFT_THUMB);
                }
                
                if(state->Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
                    state->Gamepad.wButtons &= ~(XINPUT_GAMEPAD_RIGHT_SHOULDER);
                    state->Gamepad.wButtons |= (XINPUT_GAMEPAD_RIGHT_THUMB);
                }

                if(state->Gamepad.wButtons & XINPUT_GAMEPAD_A && InsertDown == false) {
                    InsertDown = true;
                    send_key(VK_INSERT, KEYDOWN);
                    state->Gamepad.wButtons &= ~(XINPUT_GAMEPAD_A);
                }
                
                if(state->Gamepad.wButtons & XINPUT_GAMEPAD_X && F2Down == false) {
                    F2Down = true;
                    send_key(VK_F2, KEYDOWN);
                    state->Gamepad.wButtons &= ~(XINPUT_GAMEPAD_X);
                }
                
                if(state->Gamepad.wButtons & XINPUT_GAMEPAD_B && F3Down == false) {
                    F3Down = true;
                    send_key(VK_F3, KEYDOWN);
                    state->Gamepad.wButtons &= ~(XINPUT_GAMEPAD_B);
                }
                
                if(state->Gamepad.wButtons & XINPUT_GAMEPAD_Y && F4Down == false) {
                    F3Down = true;
                    send_key(VK_F4, KEYDOWN);
                    state->Gamepad.wButtons &= ~(XINPUT_GAMEPAD_Y);
                }

                // Left trigger - this is a switch LT, RB flag.
                if(state->Gamepad.bLeftTrigger >= 200 && LTDown == false) {
                    LTDown = true;
                    SwapLtRb = !SwapLtRb;
                    state->Gamepad.bLeftTrigger = 0;
                    m_VR->trigger_haptic_vibration(0.0f, 0.5f, 1.0f, 1000.0f, LeftController);	
                    m_VR->trigger_haptic_vibration(0.0f, 0.5f, 1.0f, 1000.0f, RightController);	
                }
                
                // Right trigger - this is a flag to toggle the Right Trigger haptic 
                if(state->Gamepad.bRightTrigger >= 200 && RTDown == false) {
                    RTDown = true;
                    RTHaptic = !RTHaptic;
                    state->Gamepad.bRightTrigger = 0;
                    m_VR->trigger_haptic_vibration(0.0f, 0.5f, 1.0f, 1000.0f, RightController);	
                }
                
                // L3 is often run so we will allow for up on pad and holding left menu (shift) to also engage L3
                if(state->Gamepad.sThumbLY > 5000 || state->Gamepad.sThumbLY < -5000) {
                    state->Gamepad.wButtons |= (XINPUT_GAMEPAD_LEFT_THUMB);
                }
            }
            
            if(SwapLtRb == true) {
                bool LT = (state->Gamepad.bLeftTrigger >= 200) ? true : false;
                bool RB = (state->Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? true : false;
                
                state->Gamepad.bLeftTrigger = (RB == true) ? 255 : 0;
                if(LT == true) {
                    state->Gamepad.wButtons |= (XINPUT_GAMEPAD_RIGHT_SHOULDER);
                } else {
                    state->Gamepad.wButtons &= ~(XINPUT_GAMEPAD_RIGHT_SHOULDER);
                }
            }
            
            if(RTHaptic == true) {
                if(state->Gamepad.bRightTrigger >= 200) {
                    m_VR->trigger_haptic_vibration(0.0f, 0.25f, 1.0f, 60.0f, RightController);	
                }
            }

            // Clear xinput for start & select / menu & back.
            state->Gamepad.wButtons &= ~(XINPUT_GAMEPAD_START | XINPUT_GAMEPAD_BACK);
            
            // Start is Left system + right system.
            // Back is right system alone.
            if(m_VR->is_action_active(SystemButton, RightController)) {
                if(m_VR->is_action_active(SystemButton, LeftController)) {
                    state->Gamepad.wButtons |= XINPUT_GAMEPAD_START;
                } else {
                    state->Gamepad.wButtons |= XINPUT_GAMEPAD_BACK;
                }
            }
		}
    }

};
// Actually creates the plugin. Very important that this global is created.
// The fact that it's using std::unique_ptr is not important, as long as the constructor is called in some way.
std::unique_ptr<ViveHaptic> g_plugin{new ViveHaptic()};

void DebugPrint(char* Format, ...)
{
  char FormattedMessage[512];    
  va_list ArgPtr = NULL;  
  
  /* Generate the formatted debug message. */        
  va_start(ArgPtr, Format);        
  vsprintf(FormattedMessage, Format, ArgPtr);        
  va_end(ArgPtr); 

  OutputDebugString(FormattedMessage);
}

