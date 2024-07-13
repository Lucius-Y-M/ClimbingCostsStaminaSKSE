#pragma once


#include "AliasSKSE.h"


#define TOML_EXCEPTIONS 0
#include <toml++/toml.hpp>


namespace Config {

    static Option<f32> baseCost = None;

    static Option<Pair<f32, f32>> Cutoff_And_Mult = None;


    enum ParseResult {
        kFailed,

        kBaseCostOnly,
        kCutoffAndMultOnly,
        kBaseAndCutoff
    };


    static ParseResult ParseConfig() {
        static constexpr Str CONFIG_PATH = "Data/SKSE/Plugins/ClimbingCostsStamina.toml";

        static constexpr Str ITEM_BASECOST = "fBaseCost";
        static constexpr Str ITEM_CUTOFF = "fCutoff";
        static constexpr Str ITEM_COSTMULT = "fCostMult";


        /*
            =============================================
        */


        auto parsedFile = toml::parse_file(CONFIG_PATH);

        ParseResult reporting = ParseResult::kFailed;

        if (parsedFile.failed()) {
            logger::critical("!!! CRITICAL ERROR: parsing TOML file 'ClimbingCostsStamina.toml' failed. Either the file wasn't found or it has syntax errors that prevented parsing.");
            logger::critical("!!! To save you further trouble, the DLL will not continue its initialization process.");
            logger::critical("!!! Make sure the file is correctly named, in the same (correct) folder as the DLL, and it does not contain any errors.");
            return reporting;
        }

        auto table = std::move(parsedFile);

        
        
        if (auto value_opt = table[ITEM_BASECOST]; value_opt.is_value() && value_opt.is_floating_point()) {

            auto value = value_opt.as_floating_point()->get();

            if (value >= 0.0f) {
                
                logger::info(">> Successfully read [{}] value as [{}].",
                    ITEM_BASECOST,
                    value
                );
                baseCost = value;

                reporting = ParseResult::kBaseCostOnly;
            }
            else {
                baseCost = 0.0f;
            }
        }



        {   // START OF: cutoff and mult

            Pair<f32, f32> cutoffAndMultPair = APair(-1.0f, -1.0f);

            if (auto value_opt = table[ITEM_CUTOFF]; value_opt.is_value() && value_opt.is_floating_point()) {

                auto value = value_opt.as_floating_point()->get();

                if (value >= 0.0f) {
                    
                    logger::info(">> Successfully read [{}] value as [{}].",
                        ITEM_CUTOFF,
                        value
                    );
                    cutoffAndMultPair.first = value;
                }
                else {
                    cutoffAndMultPair.first = 0.0f;
                }
            }
            if (auto value_opt = table[ITEM_COSTMULT]; value_opt.is_value() && value_opt.is_floating_point()) {

                auto value = value_opt.as_floating_point()->get();

                if (value >= 0.0f) {
                    
                    logger::info(">> Successfully read [{}] value as [{}].",
                        ITEM_COSTMULT,
                        value
                    );
                    cutoffAndMultPair.second = value;
                }
                else {
                    logger::error("-- ERROR: value for [{}] is successfully read, but is <= 0.0f, which is not acceptable!",
                        ITEM_COSTMULT
                    );
                }
            }

            if (cutoffAndMultPair.first >= 0.0f && cutoffAndMultPair.second >= 0.0f) {
                Cutoff_And_Mult = cutoffAndMultPair;

                reporting = (reporting == ParseResult::kBaseCostOnly) ? ParseResult::kBaseAndCutoff
                            : ParseResult::kCutoffAndMultOnly;
            }
        
        
        }   // END OF: cutoff and mult


        return reporting;

    }

};