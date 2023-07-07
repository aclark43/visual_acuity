//
// visual_acuity
// New task auto-generated by Insight wizard (1.0.0)
// Insight New Task Wizard, version 1.0.0
//
// Generated by Insight wizard (1.0.0).
//

#pragma once

#include <eye/protocol.hpp>
#include <eye/signal.hpp>
#include "Pest.h"
#include "visual_acuity_configuration.hpp"

namespace user_tasks::visual_acuity {
    struct Point {
        Point() {
            x = 0;
            y = 0;
        }

        Point(float X, float Y) {
            x = X;
            y = Y;
        }

        bool equal(float X, float Y)  {
            return (x == X && y == Y);
        }

        float x;
        float y;
    };

    class visual_acuity : public eye::protocol::EyerisTask<visual_acuityConfiguration>
    {
    public:
        /// Default constructor.
        explicit visual_acuity();

        /// Default destructor
        ~visual_acuity() override = default;

        /// @see EyerisTask::eventCommand()
        void eventCommand(int command, const basic::types::JSONView& arguments) override;

        /// @see EyerisTask::eventConfiguration()
        void eventConfiguration(const visual_acuityConfiguration::ptr_t& configuration) override;

        /// @see EyerisTask::eventConsoleChange()
        void eventConsoleChange(const basic::types::JSONView& change) override;

        /// @see EyerisTask::finalize()
        void finalize() final;

        /// @see EyerisTask::initialize()
        void initialize() final;

        void loadProgress();
        void loadSession();

        void DefinePlaceholderBoxes();
        void SetBoxPositions();
        void ShowBoxes();
        void HideBoxes();
        void MoveBoxesToFront();

        /// @see EyerisTask::setup()
        void setup() final;

        /// @see EyerisTask::streamAnalog()
        void streamAnalog(const eye::signal::DataSliceAnalogBlock::ptr_t& data) override;

        /// @see EyerisTask::streamDigital()
        void streamDigital(const eye::signal::DataSliceDigitalBlock::ptr_t& data) override;

        /// @see EyerisTask::streamEye()
        void streamEye(const eye::signal::DataSliceEyeBlock::ptr_t& data) override;

        /// @see EyerisTask::streamKeyboard()
        void streamKeyboard(const eye::signal::DataSliceKeyboardBlock::ptr_t& data) override;

        /// @see EyerisTask::streamJoypad()
        void streamJoypad(const eye::signal::DataSliceJoypadBlock::ptr_t& data) override;

        /// @see EyerisTask::streamMonitor()
        void streamMonitor(const eye::signal::DataSliceMonitorBlock::ptr_t& data) override;

        /// @see EyerisTask::streamMouse()
        void streamMouse(const eye::signal::DataSliceMouseBlock::ptr_t& data) override;

        /// @see EyerisTask::streamVideoCard()
        void streamVideoCard(const eye::signal::DataSliceVideoCardBlock::ptr_t& data) override;

        /// @see EyerisTask::teardown()
        void teardown() final;

        /// Janis' functions for stabilization
        /// https://gitlab.com/jintoy/eyeris-corrugationdiscrimination/-/blob/ddpi_mk2_18.12/src/corrugation_discrimination.cpp#L995
        void updateStabilizedPositions(const eye::signal::DataSliceEyeBlock::ptr_t &data);
        void resetPreviousStabilizedPositions();
        void smoothStabilizer(const eye::signal::DataSliceEyeBlock::ptr_t &data, int eyeIndex);
        void resetStabilizer(const eye::signal::DataSliceEyeBlock::ptr_t &data);

        /// Coefficients of the IIR filter
        enum {
            IIROrderA = 2,
            IIROrderB = 5
        };

    private:
        // Private definitions
        std::string int2string(int x);

        enum STATE {
            STATE_LOADING,
            STATE_TESTCALIBRATION,
            STATE_BLANK_SCREEN,
            STATE_FIXATION,
            STATE_TARGET ,
            STATE_RESPONSE,
        };
        STATE m_state;
        basic::types::JSON trialDatasave;
        basic::types::JSON recalsave;

        void gotoFixation();
        void saveData();

        // timers
        basic::time::Timer m_timer;
        basic::time::Timer m_timerCheck;
        basic::time::Timer m_timerExp;
        basic::time::Timer m_timerFixation;
        basic::time::Timer m_timerBlankScreen;
        basic::time::Timer m_timerResponse;
        basic::time::Timer m_timerTarget;
        basic::time::Timer m_timerHold;

        // time durations
        basic::time::milliseconds_t TimeFixationON;
        basic::time::milliseconds_t TimeFixationOFF;
        basic::time::milliseconds_t TimeBlankScreenON;
        basic::time::milliseconds_t TimeBlankScreenOff;
        basic::time::milliseconds_t TimeTargetON;
        basic::time::milliseconds_t TimeTargetOFF;
        basic::time::milliseconds_t TimeHoldON;
        basic::time::milliseconds_t TimeHoldOFF;
        basic::time::milliseconds_t ResponseTime;
        basic::time::milliseconds_t TimeResponseON;
        basic::time::milliseconds_t TimeResponseOFF;


        basic::time::milliseconds_t m_targetTime;
        basic::time::milliseconds_t m_blankScreenTime;
        basic::time::milliseconds_t m_fixationTime;
        basic::time::milliseconds_t m_holdTime;
        basic::time::milliseconds_t m_maskTime;
        basic::time::milliseconds_t m_respcueTime;
        basic::time::milliseconds_t m_responseTime; // how long the subject has to respond (set in params)
        basic::time::milliseconds_t m_delayTime;

    };
    // Stimuli for task
    eye::graphics::SolidPlane::ptr_t m_fixation;
    eye::graphics::SolidPlane::ptr_t m_fixationEcc;
    eye::graphics::SolidPlane::ptr_t m_whiteBox;
    eye::graphics::SolidPlane::ptr_t m_blackBox;

    eye::graphics::ImagePlane::ptr_t m_target;
    eye::graphics::ImagePlane::ptr_t m_scotoma;
    //std::vector<eye::graphics::ImagePlane::ptr_t> m_flankers;
    eye::graphics::ImagePlane::ptr_t m_flankers1;
    eye::graphics::ImagePlane::ptr_t m_flankers2;
    eye::graphics::ImagePlane::ptr_t m_flankers3;
    eye::graphics::ImagePlane::ptr_t m_flankers4;
    eye::graphics::ImagePlane::ptr_t m_num1;
    eye::graphics::ImagePlane::ptr_t m_num2;
    eye::graphics::ImagePlane::ptr_t m_num3;
    eye::graphics::ImagePlane::ptr_t m_num4;
    eye::graphics::ImagePlane::ptr_t m_arcs;


    eye::graphics::SolidPlane::ptr_t m_box1;
    eye::graphics::SolidPlane::ptr_t m_box2;
    eye::graphics::SolidPlane::ptr_t m_box3;
    eye::graphics::SolidPlane::ptr_t m_box4;
    eye::graphics::SolidPlane::ptr_t m_box_fix;

    std::string SubjectName;
    std::string  DataDestination;
    float scotomaPixelAngleSize;
    float multi_val;
    int TargetEccentricity;
    int Uncrowded;
    int FlankerType;
    int FlankerDist;
    int FlankerOrientations;
    int IsPest;
    int PestInit,pestLevel,initLevel;
    int FixationSize;
    int FixedContrast;
    int FixedTargetStrokewidth;
    int TargetStrokewidth;
    int NTrials;
    int NRecal;
    int BoxSize;
    int NFixation;
    int FixTime;
    int HoldTime;
    int TargetTime;
    int MagFactor,magnificationFactor;
    int XRes;
    int YRes;
    bool UnStab;
    int X,Y;
    int TargetImage,TargetOrientation;
    int xFlankers1;
    int xFlankers2;
    int xFlankers3;
    int xFlankers4;
    int yFlankers1;
    int yFlankers2;
    int yFlankers3;
    int yFlankers4;
    int nasal;
    int Stimulus;
    int StimulusColor;
    int BackgroundColor;
    std::vector<float> x_vectorALL;
    std::vector<float> y_vectorALL;

    int counter;

    int Increment;
    int xshift, yshift;
    int xPos,yPos;
    int TrialNumber;
    int TestCalibration;
    int ResponseFinalize;
    int Response,ResponseTime;
    int Correct;
    int gate;
    int WAIT_RESPONSE;

    int nHits;
    int nResponses;

    int m_numTestCalibration;
    int m_numCompleted;
    int m_numSession;
    int gateFix;
    int fo;

    bool fixationTrial;
    bool Badal;
    bool Mirror;
    bool scotoma;

    float pixelAngle;

    //stabilized shit
    float fixationRadius = 200;
    int currentFrameNumber;
    int X_stab, Y_stab, X_eye, Y_eye;
    int X_stab_prev, Y_stab_prev, X_eye_prev, Y_eye_prev;
    // IIR buffers
    std::vector<std::vector<float>> m_inputX;
    std::vector<std::vector<float>> m_inputY;
    std::vector<std::vector<float>> m_outputX;
    std::vector<std::vector<float>> m_outputY;
    float m_lastX = 0.0f;
    float m_lastY = 0.0f;

    bool slowStabilization = true;
    bool resetGate = true;
    bool FAKE_EYE_DATA;

    Pest* pest;
    Point loc;
}  // namespace user_tasks::visual_acuity