// Testing Notifications
#include "visual_acuity.hpp"

#include <eye/configuration.hpp>
#include <eye/messaging.hpp>

namespace user_tasks::visual_acuity {

    EYERIS_PLUGIN(visual_acuity)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Public methods
    //parameters for 30Hz filter from DPI (somewhat matches parameters for 30Hz maxflat iir from matlab)

    std::vector<float> B_IIR = {
            0.0005212926f,
            0.0020851702f,
            0.0031277554f,
            0.0020851702f,
            0.0005212926f};

    std::vector<float> A_IIR = {
            -1.8668754558f,
            0.8752161367f};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    visual_acuity::visual_acuity() :
            EyerisTask()
    {
        // Nothing to do
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void visual_acuity::eventCommand(int command, const basic::types::JSONView& arguments)
    {
        // Nothing to do
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void visual_acuity::eventConfiguration(const visual_acuityConfiguration::ptr_t& configuration)
    {
        // Nothing to do
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void visual_acuity::eventConsoleChange(const basic::types::JSONView& change)
    {
        // Nothing to do
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void visual_acuity::finalize()
    {
        if (m_numCompleted > 0)//check to see that some experiments have been conducted
        {
            char LocalDate[1024];
            time_t t = time(NULL);
            strftime(LocalDate, 1024, "%Y-%m-%d-%H-%M-%S", localtime(&t))

                    ;

            std::ostringstream fstr1;
            //fstr1 << "Data\\" << getConfiguration()->getSubjectName()<< "\\" << getConfiguration()->getSubjectName()<< "Progress.txt";

            std::ofstream out1(fstr1.str().c_str(), std::ios::app);


            if (!out1.is_open())
            {

                //info("\nProgress data file (%sProgress.txt) could not be opened for writing.  Please check that the file exists." + getConfiguration()->getSubjectName());
                //endTrial();
            }
            // keep track of the last trial played in the list and start from there in the next session
            out1 << "Recorded: " << LocalDate << std::endl;
            out1 << m_numSession << std::endl;
            out1 << Uncrowded << std::endl;
            out1 << TargetEccentricity << std::endl;
            out1 << m_numCompleted << std::endl;

        }

    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void visual_acuity::initialize()
    {
        nHits = 0;
        nResponses = 0;
        Increment = 12;
        xshift = 0;
        yshift = 0;
        xPos = 0;
        yPos = 0;
        TrialNumber = 1;
        TestCalibration = 1;
        m_numCompleted = 0;
        m_numTestCalibration = 0;
        BoxSize = 100;
        gateFix = 1;
        counter = 1;

        float pestStep = 4; //////////////////////////////////// for crt (4)
        float waldConstant = 1; // defines boundary around desired target (in trials)
        float targetP = 0.62; // use 62% for 4-afc task


        pixelAngle = getAngleConverter()->pixel2ArcminH(1.f);
        scotomaPixelAngleSize = 15.2896/pixelAngle;
        PestInit = getConfiguration() -> getPestInit();
        MagFactor = getConfiguration() -> getMagFactor();
        TargetEccentricity = round(getConfiguration() -> getTargetEccentricity()/pixelAngle);
        Uncrowded = getConfiguration() -> getUncrowded();
        UnStab = getConfiguration() -> getUnStab();
        NRecal = getConfiguration() -> getNRecal();
        NTrials = getConfiguration() -> getNTrials();
        NFixation = getConfiguration() -> getNFixation();
        FixedContrast = getConfiguration() -> getFixedContrast();
        //FixationSize = getConfiguration() -> getFixationSize()/pixelAngle;
        magnificationFactor = getConfiguration() -> getMagFactor();
        Stimulus = getConfiguration() -> getStimulus();
        StimulusColor = getConfiguration() -> getStimulusColor();
        Mirror = getConfiguration()->getMirror();
        Badal = getConfiguration()->getBadal();
        scotoma = getConfiguration()->getscotoma();

        m_fixationTime = std::chrono::milliseconds(5000); //
        m_blankScreenTime = std::chrono::milliseconds(400); // blank screen
        m_targetTime = std::chrono::milliseconds(500); //target time
        m_holdTime = std::chrono::milliseconds(getConfiguration()->getHoldTime()); //response time
        FixationSize = getConfiguration() -> getFixationSize()/pixelAngle;
        if (Stimulus == 1){m_fixation = newSolidPlane(FixationSize,FixationSize,eye::graphics::RGB(105,105,105));}
        else {m_fixation = newSolidPlane(FixationSize,FixationSize,eye::graphics::RGB(255,255,255));}

        m_fixation->setPosition(0,0);
        m_fixation->setSize(FixationSize, FixationSize);
        m_fixation->hide();

        m_fixationEcc = newSolidPlane(FixationSize,FixationSize,eye::graphics::RGB(255,255,255));
        m_fixationEcc->setPosition(TargetEccentricity ,0);
        m_fixationEcc->setSize(FixationSize, FixationSize);
        m_fixationEcc->hide();

        m_target = newImagePlane("3_20x100-2.tga");
        m_target->enableTransparency(true);
        m_target->hide();
        //m_target->setColor(eye::graphics::RGB(0, 0, 0));

        m_arcs = newImagePlane("arcs.tga");
        m_arcs->enableTransparency(true);
        m_arcs->hide();

        //scotoma Image
        m_scotoma = newImagePlane("MiniScotomas_Blur1026_5arcmin.tga");
        m_scotoma->enableTransparency(true);
        m_scotoma -> hide();


        // boxes for the recalibration trials
        m_whiteBox= newSolidPlane(FixationSize, FixationSize, eye::graphics::RGB(255, 255, 255));
        m_blackBox = newSolidPlane(FixationSize, FixationSize, eye::graphics::RGB(255,0,0)); // Change to Red AMC

        pest = new Pest(PestInit, targetP, pestStep, waldConstant, 0);

        DefinePlaceholderBoxes();
        SetBoxPositions();
        HideBoxes();

        hideAllObjects();
        startTrial();
        m_state = STATE_LOADING;
        m_timer.start(1000ms);


        srand((unsigned)time(NULL));

    }

    void visual_acuity::loadProgress()
    {
        std::ostringstream fstr1;
        // fstr1 << "Data\\" << getConfiguration()->getSubjectName()<< "\\" << getConfiguration()->getSubjectName()<< "Progress.txt";

        std::string tempStr1;
        std::string discard;
        int tempInt1, discardInt;
        std::ifstream in1(fstr1.str().c_str());

        if (!in1.is_open())
        {
            //info("\nProgress data file (%sProgress.txt) could not be opened for writing.  Please check that the file exists." + getConfiguration()->getSubjectName());
            //endTrial();
        }

        bool empty = true;

        while (in1.peek() != EOF)
        {
            in1 >> discard; //discard date line
            in1 >> tempInt1;
            in1 >> discardInt; // discard previous uncrowded/crowded
            in1 >> discardInt; // discard previous eccentricity value
            in1 >> discardInt; // discard number of previously completed trials
            empty = false;
        }

        if (empty == false)
        {
            m_numSession = tempInt1 + 1;
        }
        else
        {
            m_numSession = 1;
        }
        info("\n Starting Session: "+ std::to_string(m_numSession));


    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void visual_acuity::streamEye(const eye::signal::DataSliceEyeBlock::ptr_t& data) {
        storeUserStream("state", static_cast<int> (m_state));

//        std::vector<float> x_vector,y_vector;
//
//        for (int i = 0; i < data->size(); i++)
//        {
//            x_vector.push_back(data->get(i)->calibrated1.x());
//            y_vector.push_back(data->get(i)->calibrated1.y());
//        }
//        trialDatasave["xCal"].push_back(x_vector);
//        trialDatasave["yCal"].push_back(y_vector);


        // storeUserVariable("recal" + int2string(TrialNumber) + "Data", recalsave);

        std::shared_ptr<eye::signal::DataSliceEye> slice = data->getLatest();
        if (TargetEccentricity == 0) {
            //m_arcs->show();
            m_fixation->hide();
            m_fixationEcc->hide();
        }
        else if (TargetEccentricity < 10 && TargetEccentricity > 0)
        {
            //m_arcs->show();
            m_fixation->show();
            m_fixationEcc->hide();
        }
        else
        {
            m_arcs->hide();
            m_fixation->show();
            m_fixationEcc->hide();
        }

//        if (UnStab == true)
//        {
            X = 0;
            Y = 0;
            m_target->hide();
//        }
//        else
//        {
//            auto slice = data->getLatest();
//            X = getAngleConverter()->arcmin2PixelH(slice->calibrated1.x()) + xshift;
//            Y = getAngleConverter()->arcmin2PixelV(slice->calibrated1.y()) + yshift;
//        }





        float x, y;

        switch (m_state) {
            case STATE_LOADING:
            {
                if (m_timer.hasExpired()) {
                    gotoFixation();
                }
            }
                break;
            case STATE_TESTCALIBRATION:
            {
//                if (!m_timerCheck.hasExpired())
//                {
//                    x = getAngleConverter()->arcmin2PixelH(slice->calibrated1.x()) + xshift;
//                    y = getAngleConverter()->arcmin2PixelV(slice->calibrated1.y()) + yshift;
//
//                }
//                else
//                {

                if (!(ResponseFinalize == 1)) {
                    x = getAngleConverter()->arcmin2PixelH(slice->calibrated1.x());
                    y = getAngleConverter()->arcmin2PixelV(slice->calibrated1.y());

                    if (TargetEccentricity < 10 && TargetEccentricity >= 0) {
                        m_arcs->show();
                    }
                    m_whiteBox->setPosition(0, 0);
                    m_whiteBox->show();
                    moveToFront(m_whiteBox);
                    m_blackBox->setPosition(x + xshift + xPos, y + yshift + yPos);
                    m_blackBox->show();
                    moveToFront(m_blackBox);
                    x = 0;
                    y = 0;
                    m_fixation->hide();

                } else {

                    TestCalibration = 0;

                    xshift = xPos + xshift;
                    yshift = yPos + yshift;


                    m_blackBox->hide();
                    m_whiteBox->hide();

                    recalsave["TrialNumber"] = TrialNumber;
                    recalsave["xshift"] = xshift;
                    recalsave["yshift"] = yshift;
                    trialDatasave["xshift"] = xshift;
                    trialDatasave["yshift"] = yshift;

                    storeUserVariable("recal" + int2string(TrialNumber) + "Data", recalsave);

                    //endTrial();
                    gotoFixation();

                }
//                }
            }
                break;
            case STATE_FIXATION:
            {
                m_blackBox->hide();
                m_whiteBox->hide();
                gate = 0;
                m_fixation->setPosition(0, 0);
                m_fixation->show();
                if (scotoma == true)
                {
                    moveToBack(m_fixation);
                    slice = data->getLatest();
                    m_scotoma->setPosition(getAngleConverter()->arcmin2PixelH(slice->calibrated1.x()) + xshift,
                                             getAngleConverter()->arcmin2PixelV(slice->calibrated1.y()) + yshift); // AMC Updated
                    m_scotoma->setSize(scotomaPixelAngleSize, scotomaPixelAngleSize);
                    m_scotoma->show();
                    moveToFront(m_scotoma);
                }
                m_arcs->show();

                trialDatasave["TimeFixationON"] = TimeFixationON.count();
                trialDatasave["FrameFixationON"] = data->getLatest()->dataframeNumber;
                trialDatasave["fixationTrial"] = fixationTrial;
                //moveToFront(m_fixation);
                m_fixationEcc->setPosition(TargetEccentricity, 0);
                if (gate == 0) {
                    if (m_timerFixation.hasExpired()) {
                        TimeFixationOFF = m_timer.getTime();
                        storeUserEvent("fixationOFF");
                        trialDatasave["FrameFixationOFF"] = data->getLatest()->dataframeNumber;
                        trialDatasave["TimeFixationOFF"] = TimeFixationOFF.count();
                        hideAllObjects();
                        m_target->hide();
                        m_fixation->hide();
                        m_fixationEcc->hide();
                        if (scotoma == true)
                        {
                            m_scotoma->hide();
                        }
                        saveData();
                        gotoFixation();
                    }
                }
            }
                break;
            case STATE_BLANK_SCREEN:
            {
                if (!m_timerBlankScreen.hasExpired())
                {
                    if (TargetEccentricity < 10 && TargetEccentricity >= 0) {
                        m_arcs->show();
                        m_fixation->hide();
                    }
                    //info("showing blank screen");
                }
                else
                {
                    TimeBlankScreenOff = m_timer.getTime();
                    //TimeFixationOFF = TimeBlankScreenOff;
                    storeUserEvent("blankScreenOFF");
                    //trialDatasave["FrameFixationOFF"] = data->getLatest()->dataframeNumber;

                    info("Entering show target");

                    m_timerTarget.start(m_targetTime);
                    TimeTargetON = m_timer.getTime();
                    storeUserEvent("targetON");
                    trialDatasave["FrameTargetON"] = data->getLatest()->dataframeNumber;
                    gate = 1;
                    m_state = STATE_TARGET;

                }
            }
                break;

            case STATE_TARGET:
            {
                if ((gate == 1) && (m_timerTarget.hasExpired())) { // go to the next state
                    TimeTargetOFF = m_timer.getTime();
                    storeUserEvent("targetOFF");
                    trialDatasave["FrameTargetOFF"] = data->getLatest()->dataframeNumber;
                    trialDatasave["fixationTrial"] = 0;
                    m_target->hide();
                    info("Entering State Response");
                    info("setting wait resp to 0");
                    m_timerHold.start(m_holdTime);
                    TimeHoldON = m_timer.getTime();
                    storeUserEvent("responseON");
                    WAIT_RESPONSE = 0;
                    m_state = STATE_RESPONSE;
                    TimeResponseON = TimeHoldON ;
                    trialDatasave["FrameResponseON"] = data->getLatest()->dataframeNumber;


                } else if ((gate == 1) && (!m_timerTarget.hasExpired())) {
                    if (Uncrowded == false)
                    {
                        m_flankers1->show();
                        m_flankers2->show();
                        m_flankers3->show();
                        m_flankers4->show();



                        m_flankers1->setSize(2 * TargetStrokewidth, 10 * TargetStrokewidth);
                        m_flankers2->setSize(2 * TargetStrokewidth, 10 * TargetStrokewidth);
                        m_flankers3->setSize(2 * TargetStrokewidth, 10 * TargetStrokewidth);
                        m_flankers4->setSize(2 * TargetStrokewidth, 10 * TargetStrokewidth);

                    }
                    if (UnStab == true)
                            {

                                m_target->setPosition(X+TargetEccentricity, Y); // sk change
                                //info("I am here");
                                //m_target->setPosition(X, Y);
                                m_target->show();

                                if (Uncrowded == false)
                                    {
                                        m_flankers1->setPosition(xFlankers1,yFlankers1);
                                        m_flankers2->setPosition(xFlankers2,yFlankers2);
                                        m_flankers3->setPosition(xFlankers3,yFlankers3);
                                        m_flankers4->setPosition(xFlankers4,yFlankers4);
                                    }
                            }
                    else if (UnStab == false)
                            {
                                auto slice = data->getLatest();
                                m_target->setPosition(getAngleConverter()->arcmin2PixelH(slice->calibrated1.x()) + xshift,
                                getAngleConverter()->arcmin2PixelV(slice->calibrated1.y()) + yshift); // AMC Updated
                                //info("I am here");
                                //m_target->setPosition(X, Y);
                                m_target->show();

                                if (Uncrowded == false) {

                                    m_flankers1->setPosition(xFlankers1+getAngleConverter()->arcmin2PixelH(slice->calibrated1.x()) + xshift,
                                                             yFlankers1+getAngleConverter()->arcmin2PixelV(slice->calibrated1.y()) + yshift);
                                    m_flankers2->setPosition(xFlankers2+getAngleConverter()->arcmin2PixelH(slice->calibrated1.x()) + xshift,
                                                             yFlankers2+getAngleConverter()->arcmin2PixelV(slice->calibrated1.y()) + yshift);
                                    m_flankers3->setPosition(xFlankers3+getAngleConverter()->arcmin2PixelH(slice->calibrated1.x()) + xshift,
                                                             yFlankers3+getAngleConverter()->arcmin2PixelV(slice->calibrated1.y()) + yshift);
                                    m_flankers4->setPosition(xFlankers4+getAngleConverter()->arcmin2PixelH(slice->calibrated1.x()) + xshift,
                                                             yFlankers4+getAngleConverter()->arcmin2PixelV(slice->calibrated1.y()) + yshift);
                                    //m_flankers1->setPosition(xFlankers1+X,yFlankers1+Y);
                                }



                            }
                    //
                    moveToFront(m_target);
                    if (scotoma == true)
                    {
                        m_scotoma->setPosition(getAngleConverter()->arcmin2PixelH(slice->calibrated1.x()) + xshift,
                                               getAngleConverter()->arcmin2PixelV(slice->calibrated1.y()) + yshift); // AMC Updated
                        m_scotoma->setSize(scotomaPixelAngleSize, scotomaPixelAngleSize);
                        m_scotoma->show();
                        moveToFront(m_scotoma);
                    }
                }
            }
                break;
            case STATE_RESPONSE:
            {

                // hide all the objects
                hideAllObjects();
                m_target->hide();
                if (TargetEccentricity < 10 && TargetEccentricity >= 0) {
                    m_fixation->hide();
                }
                else {
                    m_fixation->hide();
                }
                m_arcs->hide();
                SetBoxPositions();
                if (Stimulus == 1) {
                    MoveBoxesToFront();
                    ShowBoxes();
                }

                if ((m_timerHold.hasExpired()) && (WAIT_RESPONSE == 0))
                {
                    hideAllObjects();
                    //m_arcs->hide();
                    m_target->hide();
                    info("No Response");
                    saveData();
                    TimeResponseOFF = m_timer.getTime();
                    storeUserEvent("responseOFF");
                    trialDatasave["FrameResponse"] = data->getLatest()->dataframeNumber;

                }
                else if (!m_timerHold.hasExpired() && WAIT_RESPONSE == 1)
                {
                    info("Got a Response!");
                    if (TargetImage == Response)
                    {
                        Correct = 1;
                        info("Correct!");
                    }
                    else if (Response == 1000)
                    {
                        Correct = 3;
                        info("No Response Recorded...");
                    }
                    else
                    {
                        Correct = 0;
                        info("Wrong :(");

                    }
                    //endTrial();
                    saveData();
                    TimeResponseOFF = m_timer.getTime();
                    storeUserEvent("responseOFF");
                    trialDatasave["FrameResponse"] = data->getLatest()->dataframeNumber;

                }
            }
                break;

        }
    }

    void visual_acuity::gotoFixation()
    {
        //auto slice = data->getLatest();
        info("stab {}",UnStab);
        info("\n going to fixation: " + std::to_string(TestCalibration));

        if (!(TestCalibration == 1)) {
            info("\n Trial number: " + std::to_string(TrialNumber));
        }

        IsPest = getConfiguration()->getIsPest();
        FixedTargetStrokewidth = getConfiguration()->getFixedTargetStrokewidth();
        hideAllObjects();

        if (TestCalibration == 1) {
            m_state = STATE_TESTCALIBRATION;
            info("Calib trial");
            ResponseFinalize = 0;
            m_timerCheck.start(500ms);
        }
        else {
            Correct = 3;
            Response = 1000;
            pestLevel = -1;
            //ResponseTime = 0;
            WAIT_RESPONSE = 0;
            info("done with cal");

            if (gateFix == NFixation)
            {
                fixationTrial = 1;
                gateFix = 0;
            }
            else {
                fixationTrial = 0;
                gateFix++;
            }
            info("Gate Fixation = {}", gateFix);
            if (fixationTrial) {
                //fixationTrial = 1;
                gate = 1;
//                saveData();
                m_timerFixation.start(m_fixationTime);

                TimeFixationON = m_timer.getTime();
                storeUserEvent("fixationON");
                m_state = STATE_FIXATION;
            }
            else {
                //fixationTrial = 0;
                //gateFix++;
                gate = 0;
                TargetImage = rand() % 4 + 1;
                pestLevel = pest->getTestLvl();

                if (IsPest == 1)
                {
                    if (pestLevel - floor(pestLevel) > 0)
                    {
                        info("Pest Level: "+ std::to_string(pestLevel));
                        //endTrial();
                    }
                    TargetStrokewidth = pestLevel;
                    info(("Eccentricity:"+ std::to_string(TargetEccentricity),"Target:" + std::to_string(TargetImage),"Strokewidth:" + std::to_string(TargetStrokewidth/pixelAngle)));

                }
                else
                {
                    if (Stimulus == 1){
                        TargetStrokewidth = FixedTargetStrokewidth*pixelAngle; // 1 = 20/20 line, .8 = 20/16 line
                        info(("Eccentricity:"+ std::to_string(TargetEccentricity),"Target:" + std::to_string(TargetImage),"Strokewidth:" + std::to_string(TargetStrokewidth)));

                    }
                    else{
                        TargetStrokewidth = FixedTargetStrokewidth; // 1 = 20/20 line, .8 = 20/16 line
                        info(("Eccentricity:"+ std::to_string(TargetEccentricity),"Target:" + std::to_string(TargetImage),"Strokewidth:" + std::to_string(TargetStrokewidth)));

                    }
                   // TargetStrokewidth = FixedTargetStrokewidth*pixelAngle; // 1 = 20/20 line, .8 = 20/16 line
                    //info(("Eccentricity:"+ std::to_string(TargetEccentricity),"Target:" + std::to_string(TargetImage),"Strokewidth:" + std::to_string(TargetStrokewidth)));

                }

                //info(("Eccentricity:"+ std::to_string(TargetEccentricity),"Target:" + std::to_string(TargetImage),"Strokewidth:" + std::to_string(TargetStrokewidth)));
                info("Condition {} ", Uncrowded);
                if (Stimulus == 1) {
                    if (Mirror == false & Badal == false) {
                        switch (TargetImage) {
                            case 1:
                                m_target = newImagePlane("3_20x100-2.tga");
                                break;
                            case 2:
                                m_target = newImagePlane("5_20x100-2.tga");
                                break;
                            case 3:
                                m_target = newImagePlane("6_20x100-2.tga");
                                break;
                            case 4:
                                m_target = newImagePlane("9_20x100-2.tga");
                                break;
                        }
                    }
                    else if (Mirror == false & Badal == true) {
                        switch (TargetImage) {
                            case 1:
                                m_target = newImagePlane("3_20x100-2.tga");
                                m_target->setAngle(180);
                                break;
                            case 2:
                                m_target = newImagePlane("5_20x100-2.tga");
                                m_target->setAngle(180);
                                break;
                            case 3:
                                m_target = newImagePlane("6_20x100-2.tga");
                                m_target->setAngle(180);
                                break;
                            case 4:
                                m_target = newImagePlane("9_20x100-2.tga");
                                m_target->setAngle(180);
                                break;
                        }
                    }
                    else if (Mirror == false & Badal == true) {
                        switch (TargetImage) {
                            case 1:
                                m_target = newImagePlane("3_20x100-2.tga");
                                m_target->setAngle(180);
                                break;
                            case 2:
                                m_target = newImagePlane("5_20x100-2.tga");
                                m_target->setAngle(180);
                                break;
                            case 3:
                                m_target = newImagePlane("6_20x100-2.tga");
                                m_target->setAngle(180);
                                break;
                            case 4:
                                m_target = newImagePlane("9_20x100-2.tga");
                                m_target->setAngle(180);
                                break;
                        }
                    }
                    else if (Mirror == true & Badal == true) {
                        switch (TargetImage) {
                            case 1:
                                m_target = newImagePlane("3_mirror.tga");
                                m_target->setAngle(180);
                                break;
                            case 2:
                                m_target = newImagePlane("5_mirror.tga");
                                m_target->setAngle(180);
                                break;
                            case 3:
                                m_target = newImagePlane("6_mirror.tga");
                                m_target->setAngle(180);
                                break;
                            case 4:
                                m_target = newImagePlane("9_mirror.tga");
                                m_target->setAngle(180);
                                break;
                        }
                    }
                    m_target->setSize(2 * TargetStrokewidth, 10 * TargetStrokewidth);

                    if (Uncrowded == 1)
                    {
                        FlankerDist = 0;
                        FlankerType = 0;
                    }
                    else {

                        // figure out where the flankers will be positioned (xFlanker and yFlanker contain center coordinates)
                        FlankerType = getConfiguration()->getFlankerType();
                        multi_val = rand() % 6 + 1;;//

                        RandFlankerDist = getConfiguration()-> getRandFlankerDist();
                        //FlankerDist = 1.4;
                        //FlankerOrientations = -1; // uncrowded, will change if crowded
                        //float optoDim = 5.0;
                        info("multi_val = {}", multi_val);
                        if( RandFlankerDist == true)
                        {
                            if (multi_val == 1)
                            {
                                FlankerDist = 1.85f;
                            }
                            else if (multi_val == 2)
                            {
                                FlankerDist = 1.75f;
                            }
                            else if (multi_val == 3)
                            {
                                FlankerDist = 2.0f;
                            }
                            else if (multi_val == 4)
                            {
                                FlankerDist = 1.25f;
                            }
                            else if (multi_val == 5)
                            {
                                FlankerDist = 1.15f;
                            }
                            else if (multi_val == 6)
                            {
                                FlankerDist = 200000.0f;
                            }

//                            switch (multi_val)
//                            {
//                                case 1: {FlankerDist = 0.5;}
//                                case 2: {FlankerDist = 0.75;}
//                                case 3: {FlankerDist = 1;}
//                                case 4: {FlankerDist = 1.5;}
//                                case 5: {FlankerDist = 2;}
//                            }
                        }
                        info("Flanker Dist = {}", FlankerDist);
//                        else if ( RandFlankerDist == false)
//                        {
//                            FlankerDist = 1.4;//getConfiguration()-> getFlankerDist();
//                        }

                        // case 1: // horizontal and vertical
//                if (UnStab == 1)
//                {
//
//                }
//                else {
                        xFlankers1 = -((2 * TargetStrokewidth) + X)*FlankerDist; //left
                        xFlankers2 = (2 * TargetStrokewidth + X)*FlankerDist; //right
                        xFlankers3 = (X); //bottom
                        xFlankers4 = (X); //top

                        yFlankers1 = (Y); //left
                        yFlankers2 = (Y); //right
                        yFlankers3 = (-5.f * (2 * TargetStrokewidth  + Y))*FlankerDist; //bottom
                        yFlankers4 = (5.f * (2 * TargetStrokewidth + Y))*FlankerDist; //top
                        // break;
//                }
//                case 2: // horizontal only
//                    xFlankers = (-FlankerDist+X, FlankerDist+X);
//                    yFlankers = ( Y, Y );
//                    break;
//                case 3: // vertical only
//                    xFlankers = ( X, X );
//                    yFlankers = ( -5.f*FlankerDist+Y, 5.f*FlankerDist+Y);
//                    break;
                        //}
                        FlankerOrientations = 1; // uncrowded, will change if crowded
                        //info("Successfully loaded the flanker locations");
                    }

                }
                else{
                    m_target = newImagePlane("EBarsE.tga");
                    if (Mirror == false & Badal == false) {
                        switch (TargetImage) {
                            case 1: //up
                                m_target->setAngle(90);
                                break;
                            case 2: //right
                                //m_target = newImagePlane("EBarsE.tga");
                                //m_target = newImagePlane("EBarsE.tga");
                                break;
                            case 3: //down
                                m_target->setAngle(270);
                                break;
                            case 4: //left
                                //m_target = newImagePlane("EBarsE.tga");
                                m_target->setAngle(180);
                                break;
                        }
                    }
                    else if (Mirror == true & Badal == false) {
                        switch (TargetImage) {
                            case 1: //up
                                m_target->setAngle(90);
                                break;
                            case 2: //right
                                m_target->setAngle(180);
                                break;
                            case 3: //down
                                m_target->setAngle(270);
                                break;
                            case 4: //left
                                break;
                        }
                    }
                    else if (Mirror == true & Badal == true) {
                        switch (TargetImage) {
                            case 1: //up
                                m_target->setAngle(270);
                                break;
                            case 2: //right

                                break;
                            case 3: //down
                                m_target->setAngle(90);
                                break;
                            case 4: //left
                                m_target->setAngle(180);
                                break;
                        }
                    }
                    else if (Mirror == false & Badal == true) {
                        switch (TargetImage) {
                            case 1: //up
                                m_target->setAngle(270);
                                break;
                            case 2: //right
                                m_target->setAngle(180);
                                break;
                            case 3: //down
                                m_target->setAngle(90);
                                break;
                            case 4: //left
                                break;
                        }
                    }

                    m_target->setSize(TargetStrokewidth, TargetStrokewidth);
                }

                if (UnStab == true) {
                    m_target->setPosition(0 + TargetEccentricity, 0); //sk change
                    //info("I should be here");
                    m_target->hide();
//                    if (Uncrowded == false)
//                    {
//                        m_flankers1->setPosition(xFlankers1,yFlankers1);
//                        m_flankers2->setPosition(xFlankers2,yFlankers2);
//                        m_flankers3->setPosition(xFlankers3,yFlankers3);
//                        m_flankers4->setPosition(xFlankers4,yFlankers4);
//
//                    }
                }
                else if (UnStab == false)
                {
//                    m_target->setPosition(getAngleConverter()->arcmin2PixelH(slice->calibrated1.x()) + xshift + TargetEccentricity, Y);
//                    info("NOT HERE //////");
                    m_target->hide();
//                    if (Uncrowded == false) {
//
//                        m_flankers1->setPosition(xFlankers1+X,
//                                                 yFlankers1+Y);
//                        m_flankers1->setPosition(xFlankers2+X,
//                                                 yFlankers2+Y);
//                        m_flankers1->setPosition(xFlankers3+X,
//                                                 yFlankers3+Y);
//                        m_flankers1->setPosition(xFlankers4+X,
//                                                 yFlankers4+Y);
//                        //m_flankers1->setPosition(xFlankers1+X,yFlankers1+Y);
//                    }
                }
                if (scotoma == true)
                {
                    m_scotoma->setPosition(X, Y);
                    m_scotoma->setSize(scotomaPixelAngleSize, scotomaPixelAngleSize);
                    m_scotoma->show();
                }
                if (Uncrowded == false) {
                    //int fo;
                    //FlankerOrientations = 0;
                    //info("Im here!");
                    for (int ii = 0; ii < 5; ii++) { //hard coded for 4 flankers total
                        fo = rand() % 4 + 1;
//                        while (fo == TargetOrientation) //flankers will not match the target
//                        {
//                            fo = (rand() % 3) + 1;
//                        }
                       // info("fo{}",fo);
                      //  info("ii{}",ii);

                        if (ii == 1) {
                            FlankerOrientations = 10 * FlankerOrientations +
                            fo; //save the orientation, no actual effect on the experiment
                            if (Stimulus == 1) {
                                switch (fo) {
                                    case 1:
                                        m_flankers1 = newImagePlane("3_20x100-2.tga");
                                        break;
                                    case 2:
                                        m_flankers1 = newImagePlane("5_20x100-2.tga");
                                        break;
                                    case 3:
                                        m_flankers1 = newImagePlane("6_20x100-2.tga");
                                        break;
                                    case 4:
                                        m_flankers1 = newImagePlane("9_20x100-2.tga");
                                        break;
                                }
                            }

                          //  info("past cases!");
//                            if (UnStab == true) {
//                                m_flankers1->setPosition(xFlankers1,yFlankers1);
//                            }
//                            else if (UnStab == false) {
//
//                                m_flankers1->setPosition(xFlankers1+getAngleConverter()->arcmin2PixelH(slice->calibrated1.x()) + xshift,
//                                yFlankers1+getAngleConverter()->arcmin2PixelV(slice->calibrated1.y()) + yshift);
//                                //m_flankers1->setPosition(xFlankers1+X,yFlankers1+Y);
//                            }

                            //m_flankers[ii]->setSize(2 * TargetStrokewidth, 10 * TargetStrokewidth);
                            //m_flankers[ii]->setColor(EIS_RGB(fixedContrast, fixedContrast, fixedContrast));
                            //m_flankers[ii]->pxSetPosition(TargetEccentricity + xFlankers[ii] * 2 * TargetStrokewidth,
                            //yFlankers[ii] * 2 * TargetStrokewidth);
                            //m_flankers[ii]->replaceGray(0, targetGray);
                            m_flankers1->hide();
                        }
                        else if (ii == 2) {
                            FlankerOrientations = 10 * FlankerOrientations +
                                                  fo; //save the orientation, no actual effect on the experiment
                            if (Stimulus == 1) {
                                switch (fo) {
                                    case 1:
                                        m_flankers2 = newImagePlane("3_20x100-2.tga");
                                        break;
                                    case 2:
                                        m_flankers2 = newImagePlane("5_20x100-2.tga");
                                        break;
                                    case 3:
                                        m_flankers2 = newImagePlane("6_20x100-2.tga");
                                        break;
                                    case 4:
                                        m_flankers2 = newImagePlane("9_20x100-2.tga");
                                        break;
                                }
                            }

//                            if (UnStab == true) {
//                                m_flankers2->setPosition(xFlankers2,yFlankers2);
//                            }
//                            else if (UnStab == false) {
//                                m_flankers2->setPosition(xFlankers2+getAngleConverter()->arcmin2PixelH(slice->calibrated1.x()) + xshift,
//                                yFlankers2+getAngleConverter()->arcmin2PixelV(slice->calibrated1.y()) + yshift);
//                            }

                            //m_flankers[ii]->setSize(2 * TargetStrokewidth, 10 * TargetStrokewidth);
                            //m_flankers[ii]->setColor(EIS_RGB(fixedContrast, fixedContrast, fixedContrast));
                            //m_flankers[ii]->pxSetPosition(TargetEccentricity + xFlankers[ii] * 2 * TargetStrokewidth,
                            //yFlankers[ii] * 2 * TargetStrokewidth);
                            //m_flankers[ii]->replaceGray(0, targetGray);
                            m_flankers2->hide();
                        }
                        else if (ii == 3) {
                            FlankerOrientations = 10 * FlankerOrientations +
                                                  fo; //save the orientation, no actual effect on the experiment
                            if (Stimulus == 1) {
                                switch (fo) {
                                    case 1:
                                        m_flankers3 = newImagePlane("3_20x100-2.tga");
                                        break;
                                    case 2:
                                        m_flankers3 = newImagePlane("5_20x100-2.tga");
                                        break;
                                    case 3:
                                        m_flankers3 = newImagePlane("6_20x100-2.tga");
                                        break;
                                    case 4:
                                        m_flankers3 = newImagePlane("9_20x100-2.tga");
                                        break;
                                }
                            }

//                            if (UnStab == true) {
//                                m_flankers3->setPosition(xFlankers3,yFlankers3);
//                            }
//                            else if (UnStab == false) {
//                                m_flankers3->setPosition(xFlankers3+getAngleConverter()->arcmin2PixelH(slice->calibrated1.x()) + xshift,
//                                yFlankers3+getAngleConverter()->arcmin2PixelV(slice->calibrated1.y()) + yshift);
//                            }

                            //m_flankers[ii]->setSize(2 * TargetStrokewidth, 10 * TargetStrokewidth);
                            //m_flankers[ii]->setColor(EIS_RGB(fixedContrast, fixedContrast, fixedContrast));
                            //m_flankers[ii]->pxSetPosition(TargetEccentricity + xFlankers[ii] * 2 * TargetStrokewidth,
                            //yFlankers[ii] * 2 * TargetStrokewidth);
                            //m_flankers[ii]->replaceGray(0, targetGray);
                            m_flankers3->hide();
                        }
                        else if (ii == 4) {
                            FlankerOrientations = 10 * FlankerOrientations +
                                                  fo; //save the orientation, no actual effect on the experiment
                            if (Stimulus == 1) {
                                switch (fo) {
                                    case 1:
                                        m_flankers4 = newImagePlane("3_20x100-2.tga");
                                        break;
                                    case 2:
                                        m_flankers4 = newImagePlane("5_20x100-2.tga");
                                        break;
                                    case 3:
                                        m_flankers4 = newImagePlane("6_20x100-2.tga");
                                        break;
                                    case 4:
                                        m_flankers4 = newImagePlane("9_20x100-2.tga");
                                        break;
                                }
                            }

//                            if (UnStab == true) {
//                                m_flankers4->setPosition(xFlankers4,yFlankers4);
//                            }
//                            else if (UnStab == false) {
//                                m_flankers4->setPosition(xFlankers4+getAngleConverter()->arcmin2PixelH(slice->calibrated1.x()) + xshift,
//                                yFlankers4+getAngleConverter()->arcmin2PixelV(slice->calibrated1.y()) + yshift);
//                            }
//                        else if (UnStab == false) {
//                                m_flankers4->setPosition(
//                                        X + (-TargetEccentricity) - xFlankers4,
//                                        Y + yFlankers4);
//                            }

                            //m_flankers[ii]->setSize(2 * TargetStrokewidth, 10 * TargetStrokewidth);
                            //m_flankers[ii]->setColor(EIS_RGB(fixedContrast, fixedContrast, fixedContrast));
                            //m_flankers[ii]->pxSetPosition(TargetEccentricity + xFlankers[ii] * 2 * TargetStrokewidth,
                            //yFlankers[ii] * 2 * TargetStrokewidth);
                            //m_flankers[ii]->replaceGray(0, targetGray);
                            m_flankers4->hide();
                        }
                    }
                }

                m_arcs->setSize(50 * TargetStrokewidth, 50 * TargetStrokewidth);
                gate = 1;
                m_timerBlankScreen.start(m_blankScreenTime);
                TimeBlankScreenON = m_timer.getTime();
                storeUserEvent("blankScreenON");
                m_state = STATE_BLANK_SCREEN;
            }
        }
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void visual_acuity::setup()
    {
        // Nothing to do
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void visual_acuity::streamAnalog(const eye::signal::DataSliceAnalogBlock::ptr_t& data)
    {
        // Nothing to do
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void visual_acuity::streamDigital(const eye::signal::DataSliceDigitalBlock::ptr_t& data)
    {
        // Nothing to do
    }



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void visual_acuity::streamKeyboard(const eye::signal::DataSliceKeyboardBlock::ptr_t& data) {
        auto keyboard = data->getLatest();
        if (m_state == STATE_TESTCALIBRATION) {

            if (keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_w))  // moving the cursor up
            {
                yPos = yPos + Increment; //position of the cross
            } else if (keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_z))  // moving the cursor down
            {
                yPos = yPos - Increment;
            } else if (keyboard->isKeyReleased(
                    source_keyboard::keyboard_keys_e::KEY_d))// moving the cursor to the right
            {
                xPos = xPos + Increment;

            } else if (keyboard->isKeyReleased(
                    source_keyboard::keyboard_keys_e::KEY_a)) // moving the cursor to the left
            {
                xPos = xPos - Increment;

            }

            if ((keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_x)) |
                (keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_X)))// finalize the response
            {
                info("Recalibration finalized");
                ResponseFinalize = 1;
            }

            if (m_state == STATE_RESPONSE) {
                if (Stimulus == 1) {
                    if ((keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_d)) |
                        (keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_D))) {
                        if (loc.x == 0) {
                            loc.x = -BoxSize * 2;

                            m_box1 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(255, 0, 0));
                            m_box2 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                            m_box3 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                            m_box4 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                            Response = 1;
                            //WAIT_RESPONSE = 1;
                        } else if (loc.x == BoxSize * 2) {
                            loc.x = 0;

                            m_box1 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                            m_box2 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(255, 0, 0));
                            m_box3 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                            m_box4 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                            //WAIT_RESPONSE = 1;
                            Response = 2;
                        } else if (loc.x == BoxSize * 4) {
                            loc.x = BoxSize * 2;

                            m_box1 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                            m_box2 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                            m_box3 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(255, 0, 0));
                            m_box4 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                           // W//AIT_RESPONSE = 1;
                            Response = 3;
                        }

                    } else if ((keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_a)) |
                               (keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_A))) {
                        if (loc.x == 0) {
                            loc.x = BoxSize * 2;

                            m_box1 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                            m_box2 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                            m_box3 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(255, 0, 0));
                            m_box4 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                            //WAIT_RESPONSE = 1;
                            Response = 3;

                        } else if (loc.x == BoxSize * 2) {
                            loc.x = BoxSize * 4;

                            m_box1 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                            m_box2 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                            m_box3 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                            m_box4 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(255, 0, 0));
                            //WAIT_RESPONSE = 1;
                            Response = 4;
                        } else if (loc.x == -BoxSize * 2) {
                            loc.x = 0;

                            m_box1 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                            m_box2 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(255, 0, 0));
                            m_box3 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                            m_box4 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
                            //WAIT_RESPONSE = 1;
                            Response = 2;
                        }


                    }
                    if ((keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_x)) |
                        (keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_X))) {
                        info("Answer Button Clicked");
                        if (loc.x == 0) {
                            ResponseTime = m_timer.getTime();
                            trialDatasave["ResponseTime"] = data->getLatest()->dataframeNumber;
                            Response = 2;
                            //WAIT_RESPONSE = 1;
                        } else if (loc.x == -BoxSize * 2) {
                            ResponseTime = m_timer.getTime();
                            trialDatasave["ResponseTime"] = data->getLatest()->dataframeNumber;
                            //WAIT_RESPONSE = 1;
                            Response = 1;
                        } else if (loc.x == BoxSize * 2) {
                            ResponseTime = m_timer.getTime();
                            trialDatasave["ResponseTime"] = data->getLatest()->dataframeNumber;
                            //WAIT_RESPONSE = 1;
                            Response = 3;
                        } else if (loc.x == BoxSize * 4) {
                            ResponseTime = m_timer.getTime();
                            trialDatasave["ResponseTime"] = data->getLatest()->dataframeNumber;
                            //WAIT_RESPONSE = 1;
                            Response = 4;
                        }



                        ResponseFinalize = Response;
                        loc.x = 0;

                    }
                }
                else{ //Stimulus is tumbling E
                    m_target->hide();
                    if ((keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_w)) |
                        (keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_W))) {
                        Response = 1;
                    }
                    else if ((keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_d)) |
                             (keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_D))) {
                        Response = 2;

                    }
                    else if ((keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_x)) |
                             (keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_X))) {
                        Response = 3;

                    }
                    else if ((keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_a)) |
                             (keyboard->isKeyReleased(source_keyboard::keyboard_keys_e::KEY_A))) {
                        Response = 4;

                    }

                   // WAIT_RESPONSE = 1;

                    ResponseFinalize = Response;
                    loc.x = 0;

                }

            }
        }
    }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void visual_acuity::streamJoypad(const eye::signal::DataSliceJoypadBlock::ptr_t& data) {
        auto joypad = data->getLatest();
        if (m_state == STATE_TESTCALIBRATION)
            if (Badal == false)
            {
                if (joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_UP)) // moving the cursor up
                {
                    yPos = yPos + Increment; //position of the cross
                } else if (joypad->isButtonPressed(
                        source_joypad::joypad_buttons_e::BUTTON_DOWN)) // moving the cursor down
                {
                    yPos = yPos - Increment;
                } else if (joypad->isButtonPressed(
                        source_joypad::joypad_buttons_e::BUTTON_RIGHT)) // moving the cursor to the right
                {
                    xPos = xPos + Increment;

                } else if (joypad->isButtonPressed(
                        source_joypad::joypad_buttons_e::BUTTON_LEFT)) // moving the cursor to the left
                {
                    xPos = xPos - Increment;

                }

            }
            else
            {
                if (joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_UP)) // moving the cursor up
                {
                    yPos = yPos - Increment; //position of the cross
                } else if (joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_DOWN)) // moving the cursor down
                {
                    yPos = yPos + Increment;
                } else if (joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_RIGHT)) // moving the cursor to the right
                {
                    xPos = xPos + Increment;

                } else if (joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_LEFT)) // moving the cursor to the left
                {
                    xPos = xPos - Increment;

                }

            }

        if (joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_R1)) // finalize the response
        {
            ResponseFinalize = 1;
        }

        if (m_state != STATE_RESPONSE) return;
        if (m_state == STATE_RESPONSE) {
            if (Stimulus == 1) {
                // Left
                if (joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_LEFT)) {
                    if (loc.x == 0) {
                        loc.x = -BoxSize * 2;

                        m_box2->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box3->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box4->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box1->setColor(eye::graphics::RGB(255, 0, 0));
                        Response = 1;
                    } else if (loc.x == BoxSize * 2) {
                        loc.x = 0;

                        m_box3->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box1->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box4->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box2->setColor(eye::graphics::RGB(255, 0, 0));

                        Response = 2;
                    } else if (loc.x == BoxSize * 4) {
                        loc.x = BoxSize * 2;

                        m_box4->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box1->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box2->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box3->setColor(eye::graphics::RGB(255, 0, 0));

                        Response = 3;
                    }
                }

                if (joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_RIGHT)) {
                    if (loc.x == 0) {
                        loc.x = BoxSize * 2;

                        m_box2->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box1->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box4->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box3->setColor(eye::graphics::RGB(255, 0, 0));

                        Response = 3;

                    } else if (loc.x == BoxSize * 2) {
                        loc.x = BoxSize * 4;

                        m_box3->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box1->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box2->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box4->setColor(eye::graphics::RGB(255, 0, 0));

                        Response = 4;
                    } else if (loc.x == -BoxSize * 2) {
                        loc.x = 0;

                        m_box1->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box3->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box4->setColor(eye::graphics::RGB(0, 0, 0));
                        m_box2->setColor(eye::graphics::RGB(255, 0, 0));

                        //ResponseTime = m_timer.getTime();
                        Response = 2;
                    }

                }

                if (joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_R1))// finalize the response
                {
                    info("Answer Button Clicked");
                    WAIT_RESPONSE = 0;
                    if (loc.x == 0) {
                        ResponseTime = m_timer.getTime();
                        trialDatasave["ResponseTime"] = data->getLatest()->dataframeNumber;
                        Response = 2;
                        //WAIT_RESPONSE = 1;
                    } else if (loc.x == -BoxSize * 2) {
                        ResponseTime = m_timer.getTime();
                        trialDatasave["ResponseTime"] = data->getLatest()->dataframeNumber;
                        Response = 1;
                        //W/AIT_RESPONSE = 1;
                    } else if (loc.x == BoxSize * 2) {
                        ResponseTime = m_timer.getTime();
                        trialDatasave["ResponseTime"] = data->getLatest()->dataframeNumber;
                        Response = 3;
                        //WAIT_RESPONSE = 1;
                    } else if (loc.x == BoxSize * 4) {
                        ResponseTime = m_timer.getTime();
                        trialDatasave["ResponseTime"] = data->getLatest()->dataframeNumber;
                        Response = 4;
                        // = 1;
                    }
                    //RespFinalize = Response; // click the right trigger finalize the response
                    WAIT_RESPONSE = 1; //modification by sk

                    ResponseFinalize = Response;
                    loc.x = 0;
                }
            }

            if (Stimulus == 2) {
                //Stimulus is tumbling E
                WAIT_RESPONSE = 0;
                if (joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_UP)) {
                    Response = 1;
                    //WAIT_RESPONSE = 1;
                } else if (joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_RIGHT)) {
                    Response = 2;
                    //WAIT_RESPONSE = 1;
                } else if (joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_DOWN)) {
                    Response = 3;
                    //WAIT_RESPONSE = 1;
                } else if (joypad->isButtonPressed(source_joypad::joypad_buttons_e::BUTTON_LEFT)) {
                    Response = 4;
                    //AIT_RESPONSE = 1;
                }

                WAIT_RESPONSE = 1;
                ResponseTime = m_timer.getTime();
                trialDatasave["ResponseTime"] = data->getLatest()->dataframeNumber;
                ResponseFinalize = Response;
                loc.x = 0;
            }
        }
    }


    void visual_acuity::saveData()
    {
        //
            TargetOrientation = TargetImage;
        //}

        initLevel = PestInit;


        trialDatasave["TimeFixationON"] = TimeFixationON.count();
        trialDatasave["TimeFixationOFF"] = TimeFixationOFF.count();
        trialDatasave["TimeTargetON"] = TimeTargetON.count();
        trialDatasave["TimeTargetOFF"] = TimeTargetOFF.count();
        trialDatasave["TimeResponseON"] = TimeResponseON.count();
        trialDatasave["TimeResponseOFF"] = TimeResponseOFF.count();

        trialDatasave["Correct"] = Correct;
        trialDatasave["TargetOrientation"]= TargetOrientation;
        trialDatasave["TargetEccentricity"]= TargetEccentricity;
        trialDatasave["TargetStrokewidth"]=TargetStrokewidth;
        trialDatasave["PestLevel"]= pestLevel;
        trialDatasave["Response"]= Response;
        trialDatasave["Subjects"]= getConfiguration() -> getSubjectName();
        trialDatasave["FixationSize"] = FixationSize;
        trialDatasave["InitPestLevel"] = initLevel;
        trialDatasave["pixelAngle"] = pixelAngle;
        trialDatasave["FlankerDist"] = FlankerDist;


        trialDatasave["magnificationFactor"] = magnificationFactor;
        if (Stimulus == 1){
            trialDatasave["BoxSize"] = BoxSize;
            trialDatasave["AnswerLocation"] = BoxSize*2;
        }
        else if (Stimulus == 2)
        {
            trialDatasave["AnswerOrientation"] = Response;
        }

        trialDatasave["StimulusType"] = Stimulus;
        trialDatasave["Unstabilized"] = UnStab;

        storeUserVariable("trial" + int2string(TrialNumber) + "Data", trialDatasave);

        if (Correct < 3) {
            pest->addTrial(Correct);
            nHits += Correct;
            nResponses += 1;
            int hits = pest->getHits();
            int trials = pest->getTrials();
            float perf = float(hits) / (float(trials));
            //float perf = float (nHits) / (float (nResponses));

            m_numCompleted++;
            info("Target: | Response: | Correct: | Total Perf: |");
            info("  {}    |   {}      |   {}     |  {}", TargetImage, Response, Correct, perf);
        }
        TargetImage = 0;
        fo = 0;
        m_numTestCalibration++;
        xPos = 0;
        yPos = 0;
        TrialNumber++;
        info('-----------------------------------------------------');
        TestCalibration = 1;
        gotoFixation();
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void visual_acuity::streamMonitor(const eye::signal::DataSliceMonitorBlock::ptr_t& data)
    {
        // Nothing to do
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void visual_acuity::streamMouse(const eye::signal::DataSliceMouseBlock::ptr_t& data)
    {
        // Nothing to do
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void visual_acuity::streamVideoCard(const eye::signal::DataSliceVideoCardBlock::ptr_t& data)
    {
        // Nothing to do
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void visual_acuity::teardown()
    {
        // Nothing to d
    }
    std::string visual_acuity::int2string(int x)
    {

        std::stringstream temps;
        temps << x;
        return temps.str();
    }
    void visual_acuity::DefinePlaceholderBoxes() {

        m_box1 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
        m_box2 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
        m_box3 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));
        m_box4 = newSolidPlane(BoxSize, BoxSize, eye::graphics::RGB(0, 0, 0));

        if (Mirror == true && Badal == true)
        {
            m_num1 = newImagePlane("3_mirror.tga");
            m_num1->enableTransparency(true);
            m_num1->setSize(2 * (BoxSize / 10), 10 * (BoxSize / 10));
            m_num1->setAngle(180);

            m_num2= newImagePlane("5_mirror.tga");
            m_num2->enableTransparency(true);
            m_num2->setSize(2 * (BoxSize / 10), 10 * (BoxSize / 10));
            m_num2->setAngle(180);

            m_num3 = newImagePlane("6_mirror.tga");
            m_num3->enableTransparency(true);
            m_num3->setSize(2 * (BoxSize / 10), 10 * (BoxSize / 10));
            m_num3->setAngle(180);

            m_num4 = newImagePlane("9_mirror.tga");
            m_num4->enableTransparency(true);
            m_num4->setSize(2 * (BoxSize / 10), 10 * (BoxSize / 10));
            m_num4->setAngle(180);

        }
        else
        {
            m_num1 = newImagePlane("3_20x100-2.tga");
            m_num1->enableTransparency(true);
            m_num1->setSize(2 * (BoxSize / 10), 10 * (BoxSize / 10));

            m_num2 = newImagePlane("5_20x100-2.tga");
            m_num2->enableTransparency(true);
            m_num2->setSize(2 * (BoxSize / 10), 10 * (BoxSize / 10));

            m_num3 = newImagePlane("6_20x100-2.tga");
            m_num3->enableTransparency(true);
            m_num3->setSize(2 * (BoxSize / 10), 10 * (BoxSize / 10));

            m_num4 = newImagePlane("9_20x100-2.tga");
            m_num4->enableTransparency(true);
            m_num4->setSize(2 * (BoxSize / 10), 10 * (BoxSize / 10));
        }




    }
    void visual_acuity::SetBoxPositions()
    {
        m_box1->setPosition(-BoxSize*2,0);
        m_box2->setPosition(0,0);
        m_box3->setPosition(BoxSize*2,0);
        m_box4->setPosition(BoxSize*4,0);

        m_num1->setPosition(-BoxSize*2, BoxSize*1.5);
        m_num2->setPosition(0, BoxSize * 1.5);
        m_num3->setPosition(BoxSize*2, BoxSize*1.5);
        m_num4->setPosition(BoxSize*4, BoxSize*1.5);


    }
    void visual_acuity::ShowBoxes()
    {
        if (Stimulus == 1) {
            m_box1->show();
            m_box2->show();
            m_box3->show();
            m_box4->show();

            m_num1->show();
            m_num2->show();
            m_num3->show();
            m_num4->show();
        }
    }
    void visual_acuity::HideBoxes()
    {
        m_box1->hide();
        m_box2->hide();
        m_box3->hide();
        m_box4->hide();

        m_num1->hide();
        m_num2->hide();
        m_num3->hide();
        m_num4->hide();
    }
    void visual_acuity::MoveBoxesToFront()
    {
        if (Stimulus == 1) {
            moveToFront(m_box1);
            moveToFront(m_box2);
            moveToFront(m_box3);
            moveToFront(m_box4);

            moveToFront(m_num1);
            moveToFront(m_num2);
            moveToFront(m_num3);
            moveToFront(m_num4);
        }
    }

    void visual_acuity::updateStabilizedPositions(const eye::signal::DataSliceEyeBlock::ptr_t &data) {
        for (int ii = 0; ii < 1; ii++) {

            //info("in For loop");
            X_eye_prev = X_eye;
            Y_eye_prev = Y_eye;

            X_stab_prev = X_stab;
            Y_stab_prev = Y_stab;
        }

        auto slice = data->getLatest();


        if (!FAKE_EYE_DATA) { // acquire real eye data
            if (slowStabilization) {
                // info("entered slowStab");

                smoothStabilizer(data, 0);
                X_eye = m_lastX + xshift;
                Y_eye = m_lastY + yshift;

            } else { // fast stabilization
                info("entered fast");
                X_eye = getAngleConverter()->arcmin2PixelH(slice->calibrated1.x()) + xshift;
                Y_eye = getAngleConverter()->arcmin2PixelV(slice->calibrated1.y()) + yshift;


            }
        } //else { // debug - fake an eye trace
        //float ttt = timer_state.getTime().count() / 1000.f;
        // float ttt = m_timerExp.getTime().count() / 1000.f;
        // X_eye = 15.f  * sin(2 * 3.14159 * ttt) + xshift;
        // Y_eye[0] =  15.f  * sin(2 * 3.14159 * ttt) + y_offset[0];;
        // Y_eye = 0 + yshift;// + y_origin;

//        }

        // added for stereoscopic autocalibration!!!!!
        //  for (int ee = 0; ee < nEyes; ee++) {
        //    X_eye[ee] += x_origin[ee];
        //     Y_eye[ee] += y_origin[ee];
        // }

        //  case STAB_STABILIZED
        float dX, dY, dXpos, dYpos;
        float stabgain = 0.f; // stabgain = 0 --> dXpos = dX --> image moves as eye moves (full stab)

        dX = X_eye - X_eye_prev;
        dY = Y_eye - Y_eye_prev;

        dXpos = dX * ( 1 - stabgain);
        dYpos = dY * ( 1 - stabgain);

        X_stab = X_stab_prev + dXpos;
        Y_stab = Y_stab_prev + dYpos;
    }

    void visual_acuity::resetPreviousStabilizedPositions() {

        X_eye_prev = 0;//x_origin ;
        Y_eye_prev = 0;//y_origin ;

        X_stab_prev = 0;//x_origin ;
        Y_stab_prev = 0;//y_origin ;

        X_stab = 0;
        Y_stab = 0;

    }

    void visual_acuity::resetStabilizer(const eye::signal::DataSliceEyeBlock::ptr_t &data ) {
        m_inputX.clear();
        m_inputY.clear();
        m_outputX.clear();
        m_outputY.clear();

        for (int ee = 0; ee < 1; ee++) {
            m_inputX.emplace_back();
            m_inputY.emplace_back();
            m_outputX.emplace_back();
            m_outputY.emplace_back();
        }

        auto slice = data->getLatest();
        for (auto s = 0; s < IIROrderB; s++) {
            m_inputX[0].push_back(slice->calibrated1.x());
            m_inputY[0].push_back(slice->calibrated1.y());

        }

        for (auto s = 0; s < IIROrderA; s++) {
            m_outputX[0].push_back(slice->calibrated1.x());
            m_outputY[0].push_back(slice->calibrated1.y());
        }
    }

    void visual_acuity::smoothStabilizer(const eye::signal::DataSliceEyeBlock::ptr_t &data, int eyeIndex) {
        // If the slow stabilization is enabled, then process
        // the data from the DSP through the filter and the
        // slow stabilization algorithm
        // eyeIndex = 0 (right eye), 1 (left eye)

        float X, Y;

        // If the sequence of bad data just ended, reset the
        // status of the filter to the latest available sample
        // if (CTriggers::any(Samples->triggers, Samples->samplesNumber, EOS_TRIG_1_EOE))
        //     resetFilter(Samples->x1, Samples->y1);
        // else {

        // Filter the eye movements data using a IIR
        int dsize = data->size();
        bool tracking, blinking;
        float dx, dy, x, y;
        for (int s = 0; s < dsize; s++) {
            auto slice = data->get(s);

            if (eyeIndex == 0) {
                x = slice->calibrated1.x();
                y = slice->calibrated1.y();
                tracking = slice->tracking1;
                blinking = slice->blinking1;
            } else {
                /*x = slice->calibrated2.x();
                y = slice->calibrated2.y();
                tracking = slice->tracking2;
                blinking = slice->blinking2;*/
            }

            //dx = abs(x - getAngleConverter()->pixel2ArcminH(x_origin[eyeIndex]));
            //dy = abs(y - getAngleConverter()->pixel2ArcminV(y_origin[eyeIndex]));

            dx = abs(x);
            dy = abs(y); // change for stereoscopic autocalibation

            if (tracking && !blinking && // skip if blink or no track
                (dx < fixationRadius) && // skip if not good fixation - JI: this is a hack!!!
                (dy < fixationRadius)
                    ) { // tracking signal is backwards
                // Shift the input buffers (we want to do a push_front basically)
                std::copy_backward(m_inputX[eyeIndex].begin(), m_inputX[eyeIndex].end() - 1, m_inputX[eyeIndex].end());
                std::copy_backward(m_inputY[eyeIndex].begin(), m_inputY[eyeIndex].end() - 1, m_inputY[eyeIndex].end());

                // Store the current samples
                // use calibrated traces
                m_inputX[eyeIndex][0] = x;
                m_inputY[eyeIndex][0] = y;

                // for now simulate an eye trace with noise
                /*
                m_inputX[eyeIndex][0] = 12 * cos(2 * 3.14159 * 1 * (slice->dataframeNumber) / 200) +
                              4 * cos(2 * 3.14159 * 40 * (slice->dataframeNumber) / 200) +
                              (s / 4) - 3;
                m_inputY[eyeIndex][0] = 12 * sin(2 * 3.14159 * 1 * (slice->dataframeNumber) / 200) +
                              4 * sin(2 * 3.14159 * 40 * (slice->dataframeNumber) / 200) +
                              (s / 5) + 3;
                */


                // Calculate the output of the filters
                double FilterOutputX = basic::math::dot(m_inputX[eyeIndex], B_IIR) - basic::math::dot(m_outputX[eyeIndex], A_IIR);
                double FilterOutputY = basic::math::dot(m_inputY[eyeIndex], B_IIR) - basic::math::dot(m_outputY[eyeIndex], A_IIR);

                // Shift the output buffers
                std::copy_backward(m_outputX[eyeIndex].begin(), m_outputX[eyeIndex].end() - 1, m_outputX[eyeIndex].end());
                std::copy_backward(m_outputY[eyeIndex].begin(), m_outputY[eyeIndex].end() - 1, m_outputY[eyeIndex].end());

                // Insert last samples in the output buffers
                m_outputX[eyeIndex][0] = static_cast<float>(FilterOutputX);
                m_outputY[eyeIndex][0] = static_cast<float>(FilterOutputY);
            }
        }
        // Convert the filtered eye positions from arcmins to pixels
        X = getAngleConverter()->arcmin2PixelH(m_outputX[eyeIndex][0]);
        Y = getAngleConverter()->arcmin2PixelV(m_outputY[eyeIndex][0]);

        // Perform a rounding of the position to the closest pixel
        // only if the difference between the current and the previous position
        // is above the specified threshold
        float m_sensitivity = 0.0f; // 0.75 on dpi
        if (std::abs(X - m_lastX) > m_sensitivity)
            m_lastX = static_cast<float>(std::round(X));

        if (std::abs(Y - m_lastY) > m_sensitivity)
            m_lastY = static_cast<float>(std::round(Y));

    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Protected methods

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Private methods

}  // namespace user_tasks::visual_acuity
