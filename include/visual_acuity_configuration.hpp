//
// Copyright (c) 2017-2020 Santini Designs. All rights reserved.
//

#pragma once

#include <basic/types.hpp>

#include <eye/protocol.hpp>

namespace user_tasks::visual_acuity {

/**
 * The welcome banner configuration class is configuration class for the welcome banner task.
 */
    struct visual_acuityConfiguration : public eye::protocol::EyerisTaskConfiguration
    {
        using ptr_t = std::shared_ptr<visual_acuityConfiguration>;  ///< Pointer type definition for the class

        /**
         * @brief Static factory.
         *
         * @param[in] other Pointer to another JSON object that defines the initial schema of this one.
         *
         * @return Pointer to a new class instance
         */
        [[maybe_unused]]
        static ptr_t factory_ptr(const basic::types::JSON::sptr_t& other)
        {
            return std::make_shared<visual_acuityConfiguration>(other);
        }

        /**
         * @brief Constructor that instantiate the configuration from the prototype, but then let set the values
         * also based on the parent configuration.
         *
         * @param[in] other Pointer to a JsonObject used to initialize the configuration
         */
        explicit visual_acuityConfiguration(basic::types::JSON::sptr_t other) :
                EyerisTaskConfiguration(other)


        {
            initializeSubjectName('SK');
            initializeTargetEccentricity(0);
            initializeUncrowded(true);
            initializeFlankerType(1);
            initializeFlankerDist(1.4);
            initializeIsPest(1);
            initializePestInit(12);
            initializePestLevel(1);
            initializeFixationSize(10);
            initializeFixedContrast(0);
            initializeFixedTargetStrokewidth(6);
            initializeNTrials(100);
            initializeNRecal(1);
            initializeNFixation(3);
            initializeFixTime(5000);
            initializeHoldTime(5000);
            initializeTargetTime(500);
            initializeBlankScreenTime(400);
            initializeDataDestination('Data');
            initializeMagFactor(1.0);
            initializeXRes(1920);
            initializeYRes(1080);
            initializeNHits(0);
            initializeUnStab(true);
            initializeBoxSize(120);
            initializeStimulus(1); // 1 = Pelli, 2 = Tumbling E
            initializeStimulusColor(255); // 255 = white, 0 = black
            initializeBadal(false);
            initializeMirror(false);
            initializescotoma(false);
        }
        LC_PROPERTY_INT(SUBJECT_NAME,"SubjectName", SubjectName);
        LC_PROPERTY_INT(STIMULUS,"Stimulus", Stimulus);
        LC_PROPERTY_INT(STIMULUS_COLOR,"StimulusColor", StimulusColor);
        LC_PROPERTY_INT(TARGET_ECCENTRICITY,"TargetEccentricity", TargetEccentricity);
        LC_PROPERTY_INT(UNCROWDED,"Uncrowded", Uncrowded);
        LC_PROPERTY_INT(FLANKER_TYPE,"FlankerType", FlankerType);
        LC_PROPERTY_INT(FLANKER_DIST,"FlankerDist", FlankerDist);
        LC_PROPERTY_INT(IS_PEST,"IsPest", IsPest);
        LC_PROPERTY_INT(UNSTAB,"UnStab",UnStab);
        LC_PROPERTY_INT(PEST_INIT,"PestInit", PestInit);
        LC_PROPERTY_INT(PEST_LEVEL,"PestLevel",PestLevel);
        LC_PROPERTY_INT(FIXATION_SIZE,"FixationSize", FixationSize);
        LC_PROPERTY_INT(FIXED_CONTRAST,"FixedContrast",FixedContrast);
        LC_PROPERTY_INT(FIXED_TARGET_STROKEWIDTH,"FixedTargetStrokewidth", FixedTargetStrokewidth);
        LC_PROPERTY_INT(N_Trials,"NTrials", NTrials);
        LC_PROPERTY_INT(N_Recal,"NRecal", NRecal);
        LC_PROPERTY_INT(N_Fixation,"NFixation", NFixation);
        LC_PROPERTY_INT(FIXATION_TIME,"FixTime", FixTime);
        LC_PROPERTY_INT(BLANK_SCREEN_TIME,"BlankScreenTime", BlankScreenTime);
        LC_PROPERTY_INT(HOLD_TIME,"HoldTime", HoldTime);
        LC_PROPERTY_INT(TARGET_TIME,"TargetTime", TargetTime);
        LC_PROPERTY_INT(DATA_DESTINATION,"DataDestination", DataDestination);
        LC_PROPERTY_INT(MAG_FACTOR,"MagFactor", MagFactor);
        LC_PROPERTY_INT(XRES,"XRes", XRes);
        LC_PROPERTY_INT(YRES,"YRes", YRes);
        LC_PROPERTY_INT(NHITS,"NHits", NHits);
        LC_PROPERTY_INT(NRESPONSES,"NResponses",NResponses);
        LC_PROPERTY_INT(BOX_SIZE,"BoxSize",BoxSize);
        LC_PROPERTY_INT(BADAL,"Badal",Badal);
        LC_PROPERTY_INT(MIRROR,"Mirror",Mirror);
        LC_PROPERTY_INT(SCOTOMA,"scotoma",scotoma);

    };

}  // namespace user_tasks::visual_acuity