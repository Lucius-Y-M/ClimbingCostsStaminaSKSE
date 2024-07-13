#pragma once

#include "AliasSKSE.h"
#include "Config.h"


using Furniture = RE::TESFurniture;

namespace EVGATForms {


    static constexpr inline Str EVGAT_PLUGIN_NAME = "EVGAnimatedTraversal.esl";

    static Pair<u32, Furniture *> marker01 = APair(0x80A, nullptr);
    static Pair<u32, Furniture *> marker02 = APair(0x812, nullptr);
    static Pair<u32, Furniture *> marker03 = APair(0x827, nullptr);
    static Pair<u32, Furniture *> marker04 = APair(0x83E, nullptr);
    static Pair<u32, Furniture *> marker05 = APair(0x891, nullptr);
    static Pair<u32, Furniture *> marker06 = APair(0x8F4, nullptr);
    static Pair<u32, Furniture *> marker07 = APair(0x920, nullptr);
    static Pair<u32, Furniture *> marker08 = APair(0x956, nullptr);
    static Pair<u32, Furniture *> marker09 = APair(0x982, nullptr);
    static Pair<u32, Furniture *> marker10 = APair(0x98A, nullptr);
    static Pair<u32, Furniture *> marker11 = APair(0x992, nullptr);
    static Pair<u32, Furniture *> marker12 = APair(0x99A, nullptr);
    static Pair<u32, Furniture *> marker13 = APair(0x9BF, nullptr);
    static Pair<u32, Furniture *> marker14 = APair(0xF1D, nullptr);
    static Pair<u32, Furniture *> marker15 = APair(0xF43, nullptr);

    static Array<Pair<u32, Furniture *> *, 15> EVGAT_MARKERS = {
        & marker01,
        & marker02,
        & marker03,
        & marker04,
        & marker05,
        & marker06,
        & marker07,
        & marker08,
        & marker09,
        & marker10,
        & marker11,
        & marker12,
        & marker13,
        & marker14,
        & marker15
    };




};


    static f32 (* LogicFuncPtr) (void) = nullptr;


class Hook {


private:


    struct PatchStruct_Furniture {

        static bool thunk(

            RE::TESFurniture * a_this,

            RE::TESObjectREFR* a_targetRef,
            RE::TESObjectREFR* a_activatorRef,
            
            [[maybe_unused]]
            u8 a_arg3,

            [[maybe_unused]]
            RE::TESBoundObject* a_object,

            [[maybe_unused]]
            i32 a_targetCount

        ) {

            if (
                a_this &&
                a_activatorRef
                && a_activatorRef->IsPlayerRef()
                && a_activatorRef->Is3DLoaded()
                && a_targetRef
            ) {

                if (auto * baseObj = a_targetRef->GetBaseObject(); baseObj) {

                    


                    for (auto & p : EVGATForms::EVGAT_MARKERS) {

                        auto [fid, ptr] = *p;

                        if (p->second == baseObj) {

                            auto player = RE::PlayerCharacter::GetSingleton();

                            /*
                                calculate here
                            */

                            f32 dmgVal = LogicFuncPtr();
                            f32 currStamina = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);

                            /*
                                enough stamina to climb, let pass
                            */
                            if (dmgVal <= currStamina) {
                                player
                                ->AsActorValueOwner()
                                ->RestoreActorValue(
                                    RE::ACTOR_VALUE_MODIFIER::kDamage,
                                    RE::ActorValue::kStamina,
                                    - dmgVal
                                );
                            }
                            else {
                                return false;
                            }


                            break;
                        }
                    }

                }

            }

            return func(
                a_this,



                a_targetRef,
                a_activatorRef,
                
                a_arg3,
                a_object,
                a_targetCount
            );
        }

        static inline REL::Relocation<decltype(&thunk)> func;


        static constexpr inline usize size = 0x37;
    };



    static bool InitializeForms() {
        auto * dataHandler = RE::TESDataHandler::GetSingleton();
        if (dataHandler == nullptr) { return false; }


        auto & arr1 = EVGATForms::EVGAT_MARKERS;


        for (auto & p : arr1) {
            p->second = dataHandler->LookupForm<Furniture>(p->first, EVGATForms::EVGAT_PLUGIN_NAME);

            if (p->second == nullptr) {
                logger::error("!! ERROR: one of the activators from plugin 'EVGAnimatedTraversal.esl' cannot be initialized. Either the ESL plugin is missing / inactive, or it is a newer version unexpected by this DLL.");
            }

            else {
                logger::debug("~~ EVGAT FURN with FID = [{}] and EDID = [{}] initialized.",
                    p->first,
                    p->second->GetFormEditorID()
                );
            }
        }

        bool AllItemsInitialized = true;
        for (auto & p : arr1) {
            if (p->second == nullptr) {
                AllItemsInitialized = false;
                break;
            }
        }

        if (AllItemsInitialized == false) {
            logger::error("!! ERROR: not even one EVG Animated Traversal Activators detected. EVGAT / SkyClimb hook will fail to work.");
            logger::error("!! EVGAT Hook will not be installed.");
            
            return false;
            
        }

        return true;
    }


    struct LogicFuncs {
                static constexpr inline f32 DEFAULT = 0.0f;

                static inline f32 Calculate_BaseCostOnly() {
                    return Config::baseCost.value_or(DEFAULT);
                }

                static inline f32 Calculate_MultOnly() {
                    f32 invWgt = RE::PlayerCharacter::GetSingleton()->GetWeightInContainer();

                    if let_some(
                        Config::Cutoff_And_Mult,

                        [cutoff COMMA mult],
                        {
                            if (invWgt < cutoff) {
                                return DEFAULT;
                            }

                            return (invWgt - cutoff) * mult;
                        }
                    )
                    else {
                        return DEFAULT;
                    }
                    
                }

                static inline f32 Calculate_BaseAndMult() {
                    return Calculate_BaseCostOnly() + Calculate_MultOnly();
                }
    };




public:

    static bool Install() {

        logger::debug("-- EVGAT / SkyClimb Hook installing...");


        logger::info("-- 1. Parsing Config file...");


        using ParseRes = Config::ParseResult;
        switch (Config::ParseConfig()) {
            case ParseRes::kBaseCostOnly :  {
                LogicFuncPtr = LogicFuncs::Calculate_BaseCostOnly;
                break;
            }

            case ParseRes::kCutoffAndMultOnly : {
                LogicFuncPtr = LogicFuncs::Calculate_MultOnly;
                break;
            }

            case ParseRes::kBaseAndCutoff : {
                LogicFuncPtr = LogicFuncs::Calculate_BaseAndMult;
                break;
            }

            default : {
                logger::error(">> Neither 'BaseCost' nor 'Cutoff + CostMult' is defined. DLL will not work.");
                return false;
            }
        }


        logger::info("-- 2. Initialize Forms from [{}]...",
            EVGATForms::EVGAT_PLUGIN_NAME
        );

        if (InitializeForms() == false) {
            return false;
        }


        logger::info("-- 3. Install hook...");


        static constexpr usize VTABLE_IDX = 0;

        stl::write_vfunc<
            Furniture,
            VTABLE_IDX,
            PatchStruct_Furniture
        >();


        logger::debug("-- EVGAT (Everglaid Animated Traversal) Hook installed. DLL is now functional.");

        return true;
    }




};