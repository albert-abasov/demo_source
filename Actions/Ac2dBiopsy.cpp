​////////////////////////////////////////////////////////////////////////////
// 
// COPYRIGHT KONINKLIJKE PHILIPS ELECTRONICS N.V. 2011
// All rights are reserved. Reproduction in whole or in part is
// prohibited without the written consent of the copyright owner.
//
////////////////////////////////////////////////////////////////////////////


#include <stdafx.h>

//--------------------------------------------------------------------------
// (1) Include the immediate header file
//--------------------------------------------------------------------------
#include <Ac2dBiopsy.h>

//--------------------------------------------------------------------------
// (2) Include local/private headers for this project/domain
//--------------------------------------------------------------------------
#include <AcPercuNavBase.h>
#include <ActionRegisterRequest.h>
#include <ScanHeadMgr.h>
#include <CMyUlsiMainScanheadClient.h>
#include <CMyUlsiDataModeControlClient.h>
#include <CMyUlsiDataLocationOfInterestClient.h>
#include <CMyAprMainAcquisitionClient.h>
#include <CMyAprMainProtocolClient.h>
#include <TBArbitrationMgr.h>
#include <DlgMgr.h>
#include <OmniProbe.h>
//--------------------------------------------------------------------------
// (3) Include public headers for this domain
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// (4) Include public headers for other domains
//--------------------------------------------------------------------------
#include <percunav/PercuNavMainControlClient.h>
#include <CMyUlsiMainTransactionClient.h>
#include <dms/dmsInterfaces.h>
#include <dms/dmsCapConfigParamId.h>
#include <rids/ridsSingleton.h>

//--------------------------------------------------------------------------
// (5) Include external, third-party headers
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// (6) Static definitions local to this file
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// (7) Use other namespaces (sparingly) and/or individual symbols from them
//--------------------------------------------------------------------------
USING_NS_UIDM
DPS_DECLARE_PRINT_CONTROL(bridges::DOMAIN_BASE_ID_APPUI, CAc2DBiopsy, NUM_OF_BIOPSYGUIDES);
//--------------------------------------------------------------------------
// (8) The rest, method/function definitions, etc.
//--------------------------------------------------------------------------

BEGIN_NS_EVTMGR

namespace
{
    CActionRegisterRequest<CAc2DBiopsy>
        s_Reg2DBiopsy(ACTIONID(EVTMGR_ACTION_2D_BIOPSY));
}

//============================================================================
CAc2DBiopsy::CAc2DBiopsy(CActionId actionId)
    :
    CSimpleAction( actionId ),
    CEPMData( actionId),
    m_nBiopsyIsAllowed(0),
    m_nNum1Angles(-1),
    m_nNum2Angles(-1),
    m_biopsyType1(BIOPSY_TYPE_UNDEFINED),
    m_biopsyType2(BIOPSY_TYPE_UNDEFINED),
    m_selectedBiopsyType(BIOPSY_TYPE_UNDEFINED),
    m_bUserHasSelectedBiopsyType(false),
    m_bSkipAngleSelection(false),
    m_bDisplayGuideSelectionDlg(false),
    m_bDisplayVerzaSpecificDlg(false),
    m_nNumAngles(-1),
    m_nAngleIndex(-1),
    m_bDisplayWarningPopup(false),
    m_bIsInterventionTSP(false),
    m_biopsyGuide (BIOPSY_MULTIPLE_GUIDE_WARNING_NOWARNING),
    m_bIsScanheadMotorized(false)
{
    m_bDisableVerzaInSelection = !(dms::CDmsInterfaces::getInstance().getCapConfigBoolValue(INT_CAP_FULLBIOPSY));
}

//============================================================================
CAc2DBiopsy::~CAc2DBiopsy() = default;

//============================================================================
void CAc2DBiopsy::resetEpmDataFlags()
{
    //reset all flags
    m_bDisplayVerzaSpecificDlg = false;
    m_bDisplayGuideSelectionDlg = false;
    m_bDisplayWarningPopup = false;
    m_nBiopsyIsAllowed = 0;
    m_nNum1Angles = -1;
    m_nNum2Angles = -1;
    m_nNumAngles = -1;
    m_biopsyType1 = BIOPSY_TYPE_UNDEFINED;
    m_biopsyType2 = BIOPSY_TYPE_UNDEFINED;
    m_nAngleIndex = -1;
    m_biopsyGuide = BIOPSY_MULTIPLE_GUIDE_WARNING_NOWARNING;
    m_selectedBiopsyType = BIOPSY_TYPE_UNDEFINED;
    m_bIsScanheadMotorized = false;
}

//============================================================================
bool evtmgr::CAc2DBiopsy::isOptionedForPercuNavMultiBracket()
{
    return dms::CDmsInterfaces::getInstance().getCapConfigBoolValue(INT_CAP_PERCUNAVMULTIBRACKET);
}

//============================================================================
void CAc2DBiopsy::updateEPMData()
{
    resetEpmDataFlags();

    // The number of biopsy guides can be up to two and the number of angles per
    // biopsy guide can be up to four.
    getTagValue( EPM_CMN_BIOPSY_BIOPSYNUMGUIDESALLOWED, m_nBiopsyIsAllowed);

    getTagValue( EPM_CMN_BIOPSY_BIOPSY1NUMANGLES, m_nNum1Angles);
    getTagValue(EPM_CMN_BIOPSY_BIOPSY2NUMANGLES, m_nNum2Angles);
    
    getTagValue(EPM_CMN_BIOPSY_BIOPSY1TYPE, m_biopsyType1);
    getTagValue(EPM_CMN_BIOPSY_BIOPSY2TYPE, m_biopsyType2);

    getTagValue(EPM_CMN_SH_ISMOTORIZED, m_bIsScanheadMotorized);
   
    // suppress Biopsy button for China release when only Verza is available, this only happens when biopsy type is Verza and number of biopsy guide is 1
    if (m_nBiopsyIsAllowed == 1 && m_biopsyType1 == static_cast<int>(BIOPSY_TYPE_VERZA) && m_bDisableVerzaInSelection)
    {
        m_nBiopsyIsAllowed = 0;
    }

    int biopsyGuideMsg;
    getTagValue( EPM_ECHO_TWOD_BIOPSYMULTIPLEGUIDEWARNING, biopsyGuideMsg);
    m_biopsyGuide = static_cast<EBiopsyMultipleGuideWarning>(biopsyGuideMsg);

    // switch to UltraProII for China release when Verza and UltraProII are available
    if (m_bDisableVerzaInSelection && m_nBiopsyIsAllowed == 2 && m_biopsyGuide == BIOPSY_MULTIPLE_GUIDE_WARNING_TWO_GUIDES_ULTRAPRO_AND_VERZA)
    {
        assert(m_bDisplayGuideSelectionDlg == false);
        // do nothing, China release with UltraProII and Verza in EPM database but only show UltraProII
    }
    else
    {
        if (m_biopsyGuide == BIOPSY_MULTIPLE_GUIDE_WARNING_TWO_GUIDES_ULTRAPRO_AND_VERZA)
        {
            m_bDisplayGuideSelectionDlg = true;
        }
        else if (m_biopsyGuide == BIOPSY_MULTIPLE_GUIDE_WARNING_TWO_GUIDES_INFINITI_AND_VERZA)
        {
            //Set Verza specific for now, then check below for override
            m_bDisplayVerzaSpecificDlg = true;

            // If percunav or AIbreast license is on and we are using L12-5, force selection dlg
            // We do this because Percunav must have user select to load proper configuration when procedure starts or while in a procedure
            if (isOptionedForPercuNavMultiBracket()
                && (CAcPercuNavBase::IsPercuNavLicensedOptionOn() || CAcPercuNavBase::IsAIBreastLicensedOptionOn()))
            {
                const unsigned int nScanHeadPort = static_cast<unsigned int>(CScanHeadMgr::Instance().getSelectedPortId());
                if (nScanHeadPort != 0)
                {
                    CMyUlsiMainScanheadClient    usiMainShClient;
                    const int nPromId = usiMainShClient->connectedScanhead(nScanHeadPort).getValue(usiMainShClient.getTimeout());

                    if (nPromId == static_cast<int>(SCANHEAD_SH_ID_L12_5))
                    {
                        // Override Verza specific in this case and instead force selection dialog
                        m_bDisplayGuideSelectionDlg = true; // show selection dialog
                        m_bDisplayVerzaSpecificDlg = false; // Don't show Verza warning
                        m_biopsyGuide = static_cast<EBiopsyMultipleGuideWarning>(BIOPSY_MULTIPLE_GUIDE_WARNING_TWO_GUIDES_ULTRAPRO_AND_VERZA); //Switch from infinity to UProII
                    }
                }
            }
        }
        else if (m_biopsyGuide != BIOPSY_MULTIPLE_GUIDE_WARNING_NOWARNING)
        {
            m_bDisplayWarningPopup = true;
        }
        else
        {
            // do nothing, which is the default UltraProII only case
        }
    }

    // Retrieve if it is interventional TSP
    getTagValue( EPM_FEGBL_FEMODE_ISINTERVENTIONTSP, m_bIsInterventionTSP );

    if (m_bDisplayGuideSelectionDlg)
    {
        // If the user will select the type then set undefined
        m_selectedBiopsyType = BIOPSY_TYPE_UNDEFINED;
    }
    else
    {
        //by default selected type is type 1
        m_selectedBiopsyType = m_biopsyType1;
    }
    //by default number of angles is number of angles for type 1
    m_nNumAngles = m_nNum1Angles;

}

//============================================================================
void CAc2DBiopsy::perform( const CEMActionParams& params,  CEMTSEventData &)
{
    
    if (params.getParamsSize() > 0
        && params.getParamByIndex(0).compare("START_PROC") == 0)
    {
        // This indicates we got a button press to start the AIBreast procedure
        doWorkForImmediateHandling(params, true);
    }
    else
    {
        // This is a button press on the biopsy guide button
        biopsyButtonPressed(params);
    }
}

void evtmgr::CAc2DBiopsy::biopsyButtonPressed(const CEMActionParams& params)
{
    // Delayed handling scenario: 
    //   If biopsy guide button is being pressed, then we either display the predetermined dialog 
    //   or just turn on the guide right now
    if (m_bDisplayVerzaSpecificDlg && m_bDisableVerzaInSelection)
    {
        // Don't do anything, this should not be reached
    }
    else if (m_bDisplayVerzaSpecificDlg && !m_bDisableVerzaInSelection)
    {
        CDlgMgr::Instance().showVerzaSpecificGraphicsWarningDlg();
    }
    else if (m_bDisplayGuideSelectionDlg)
    {
        const bool bDlgWithForcedSelection = CAcPercuNavBase::isPercuNavOrAIBreastActiveOrPaused();
        CDlgMgr::Instance().showBiopsyGuideSelectionDlg(bDlgWithForcedSelection);
    }
    else if(m_bDisplayWarningPopup)
    {   
        // The popup window shows any warning messages associated with the biopsy guide for the current probe.
        // For example: C8-5
        CDlgMgr::Instance().showBiopsyWarningDialog(m_biopsyGuide);
    }
    else
    {
        refreshTSControl( "EVTMGR_ACTION_2D_BIOPSY_ANGLE" );
        doWorkForDelayedHandling( params);
    }
}

//============================================================================
void CAc2DBiopsy::perform(const CEMActionParams& params, CEMEventData &)
{
    // One of the guide selection or warning dialogs were just dismissed, we need to now apply it's effect
    const bool bDismissed = CommMgr::Instance().GetBiopsyGuideSelectionDlgIf()->isDismissed();
    if (!bDismissed  && m_bDisplayGuideSelectionDlg)
    {
        //a selection dialog was displayed and user made a selection. Depending on the selection
        //we need to use the number of angles.

        //Check whether Verza was selected
        const bool isVerzaGuide = CommMgr::Instance().GetBiopsyGuideSelectionDlgIf()->isVerzaGuideSelected();
        if (isVerzaGuide)
        {
            m_selectedBiopsyType = BIOPSY_TYPE_VERZA;
            //select the number of angles for Verza
            m_nNumAngles = (m_biopsyType1 == static_cast<int>(BIOPSY_TYPE_VERZA)) ? m_nNum1Angles : m_nNum2Angles;
        }
        else
        {   //ultra pro 11 was selected, get the number of angles for that guide.
            m_selectedBiopsyType = BIOPSY_TYPE_ULTRA_PRO_II;
            m_nNumAngles = (m_biopsyType1 == static_cast<int>(BIOPSY_TYPE_ULTRA_PRO_II)) ? m_nNum1Angles : m_nNum2Angles;
        }
        m_bUserHasSelectedBiopsyType = true; // user has made a selection, remember that to avoid asking again later
    }

    if (m_bDisplayWarningPopup || m_bDisplayVerzaSpecificDlg || m_bDisplayGuideSelectionDlg)
    {
        // The warning dialog was Ok'd and a general event was sent for us to do our work
        // So we close the warning dialog and do it
        CDlgMgr::Instance().closeBiopsyWarningDialog();
    }

    if (!bDismissed)
    {
        // we handled the dialog, so reset that a dialog is required
        m_bDisplayVerzaSpecificDlg = false;
        m_bDisplayGuideSelectionDlg = false;
        m_bDisplayWarningPopup = false;
        refreshTSControl( "EVTMGR_ACTION_2D_BIOPSY_ANGLE" );

        if (!m_bSkipAngleSelection)
        {
            // This is the delayed handling scenario:
            //    perform setting the angle for the biopsy guide
            doWorkForDelayedHandling(params);  
        }
        else
        {
            //This is the immediate handling scenario:
            //    sync the internal information to ulsi, and send same data to PN
            syncInternalDataToULSIAndPercuNav(false);
            m_bSkipAngleSelection = false;
        }
    }
}

////////////////////////////////////////////////////////////////////////////
// perform :
//
// This handles internal event (e.g., EVTMGR_INTERNAL_BIOPSY_OFF)
//in: evtData: data that contains the internal event.
//============================================================================
void CAc2DBiopsy::perform( const CEMActionParams& params, CEMInternalEventData & evtData )
{
    bool bImmediateHandling = true;
    bool bProcStarting = false;
    CEMInputEventCode code = evtData.getEventCode();
    if (code == "EVTMGR_INTERNAL_CP_DYNAMIC_BIOPSY_PUSH")
    {
        biopsyButtonPressed(params);
    }
    else
    {
        if (evtData.getEventCode().compare(CEMInputEventCode("EVTMGR_INTERNAL_PERCUNAV_PATIENT_INFO_CHECK_SET_BIOPSY_DATA")) == 0)
        {
            // This is immediate handling scenario:
            // we get this after a scanhead reconnect when procedure starts
            // we want to simply reapply the passed in data
            typedef std::tuple<bool, EBiopsyType, int, bool> tpl;

            CEMInternalEventData2<pitk::NarrowReferenceCountedString, CEMInternalEventData1<tpl>>* data2 = dynamic_cast<CEMInternalEventData2<pitk::NarrowReferenceCountedString, CEMInternalEventData1<tpl>>*>(&evtData);


            EBiopsyType eGuideType = BIOPSY_TYPE_UNDEFINED;
            int bAngleIndex = -1;
            bool bCTOnlyProc = false;

            if (data2)
            {
                bool bGuideState = false;
                CEMInternalEventData1<tpl>* internalEventData1 = data2->getParam2();
                if (internalEventData1)
                {
                    std::tie(bGuideState, eGuideType, bAngleIndex, bCTOnlyProc) = *(internalEventData1->getParam1());
                }
            }
            else
            {
                uiLogNoEHAandThrowAnticipatedFault(APPUI_ERROR_LOC(L"CAc2DBiopsy::perform( const CEMActionParams& params, CEMInternalEventData & evtData )", 1), eha::APPUI_FLT_UNEXPECTED_EVENT_DATA, L"passed in evtData doesn't have expected data");
            }

            m_bUserHasSelectedBiopsyType = eGuideType != BIOPSY_TYPE_UNDEFINED;
            m_selectedBiopsyType = eGuideType;
            m_nAngleIndex = bAngleIndex;

            if (m_bUserHasSelectedBiopsyType)
            {
                //make sure dialogs don't pop up when guide button pressed
                m_bDisplayVerzaSpecificDlg = false;
                m_bDisplayGuideSelectionDlg = false;
                m_bDisplayWarningPopup = false;
            }
            // now that we have all out internal info correct
            //    sync the internal information to ulsi, and send same data to PN
            syncInternalDataToULSIAndPercuNav(false);

            bImmediateHandling = isOptionedForPercuNavMultiBracket() && !bCTOnlyProc; // don't ask user immediately if CTOnly proc
            bProcStarting = true; // we know proc is starting but PercuNav.On has not transitioned yet


        }
        else if (params.getParamsSize() > 0 &&
            params.getParamByIndex(0).compare("deselect") == 0)
        {
            // In either immediate or delayed handling scenario we handle deselect the same
            //    This is a request to turn the biopsy guide off
            doWorkForDelayedHandling(params);

            // update display of Biopsy-related TS controls
            refreshTSControl("EVTMGR_ACTION_2D_BIOPSY_ANGLE");
            refreshTSControl(getActionId());

            //Note: we skip immediate here to avoid re-enabling
            bImmediateHandling = false;
        }
        else if (params.getParamsSize() > 0
            && (params.getParamByIndex(0).compare("FORCE_SELECTION") == 0
                || params.getParamByIndex(0).compare("START_PROC") == 0
                || params.getParamByIndex(0).compare("START_PROC_NOTIFY") == 0
                || params.getParamByIndex(0).compare("FORGET_SELECTION") == 0))
        {
            // if not optioned for PN multiBracket, then we ignore these calls
            bImmediateHandling = isOptionedForPercuNavMultiBracket();

            // If request was force selection, then this was for transducer select, we need to update PN with data
            if (!bImmediateHandling
                && (params.getParamByIndex(0).compare("FORCE_SELECTION") == 0
                    || params.getParamByIndex(0).compare("FORGET_SELECTION") == 0))
            {
                syncInternalDataToULSIAndPercuNav(false);
            }

        }

        if (bImmediateHandling && m_nBiopsyIsAllowed > 0)
        {
            doWorkForImmediateHandling(params, bProcStarting);
        }

        DPS_BEGIN(NUM_OF_BIOPSYGUIDES)
        {
            if (m_nBiopsyIsAllowed > 0)
            {
                DPS_PRINT(NUM_OF_BIOPSYGUIDES, L"CAc2DBiopsy::perform CEMInternalEventData CALLED");
            }
        }
        DPS_END(NUM_OF_BIOPSYGUIDES)
    }
}

void evtmgr::CAc2DBiopsy::doWorkForImmediateHandling(const CEMActionParams &params, bool bProcStarting)
{
    CMyUlsiMainScanheadClient usiMainShClient;
    const bool bTrackingHardwarePresent = usiMainShClient->isTrackingHardwarePresent().getValue(usiMainShClient.getTimeout());

    bool bForceSelection = false;
    bool bForgetSelection = false;
    if (params.getParamsSize() > 0)
    {
        bForceSelection = params.getParamByIndex(0).compare("FORCE_SELECTION") == 0;
        bForgetSelection = params.getParamByIndex(0).compare("FORGET_SELECTION") == 0;
    }

    if (bForceSelection || bForgetSelection)
    {
        // if the event wants to force or forget selection, then we forget if the user picked before
        m_bUserHasSelectedBiopsyType = false;

        // if a selection is required set to undefined to force selection, otherwise set to default
        m_selectedBiopsyType = (!bTrackingHardwarePresent  && m_biopsyGuide == BIOPSY_MULTIPLE_GUIDE_WARNING_TWO_GUIDES_ULTRAPRO_AND_VERZA) ? BIOPSY_TYPE_UNDEFINED : m_biopsyType1;

        syncInternalDataToULSIAndPercuNav(false);
    }

    // If EPM tells us there are multiple guide types and the user hasn't selected one, display dialog
    // unless this is an integrated tracker, if so leave it up biopsy guide on to show dlg since PN dopesnt need to force selection
    if (!bForgetSelection
        && m_biopsyGuide == BIOPSY_MULTIPLE_GUIDE_WARNING_TWO_GUIDES_ULTRAPRO_AND_VERZA
        && !m_bUserHasSelectedBiopsyType
        && !bTrackingHardwarePresent)
    {
        // Set a flag so we know not to apply the angle after the dialog
        m_bSkipAngleSelection = true;
        const bool bDlgWithForcedSelection = bProcStarting || CAcPercuNavBase::isPercuNavOrAIBreastActiveOrPaused();
        CDlgMgr::Instance().showBiopsyGuideSelectionDlg(bDlgWithForcedSelection);
    }
    else if (params.getParamsSize() > 0
        && params.getParamByIndex(0).compare("START_PROC") == 0)
    {
        // Starting proc, not forcing dlg on startup, send current data
        const bool bDontCheckState = true; // don't check state since we are transitioning, only check license
        const bool bTurningBiopsyGuideOff = false; // we know we arent turning off the guide here
        SendDataToPercuNav(bTurningBiopsyGuideOff, bDontCheckState);
    }
}

//============================================================================
void CAc2DBiopsy::doWorkForDelayedHandling( const CEMActionParams& params )
{
    bool bDeselect = false;

    CMyUlsiMainScanheadClient       ulsiMSClient;
    pitk::CPitkFutureClient<bool> ulsiCtrl(ulsiMSClient->getBiopsyGuideState());
    m_data.value = ulsiCtrl.getValue(ulsiMSClient.getTimeout());
    
    pitk::CPitkFutureClient<int> ulsiContrl(ulsiMSClient->getBiopsyAngleIndex());
    m_nAngleIndex = ulsiContrl.getValue(ulsiMSClient.getTimeout());
    
    if(m_nNumAngles == 1 && !m_bIsInterventionTSP && !m_bSkipAngleSelection)
    {
        //Only Angle A, which will not be shown on TS for User to click.
        // except for Internvention TSP
        m_nAngleIndex = 0;
    }

    bool bTurningBiopsyGuideOff = false; //assume we are not forcing the guide off

    if(m_nAngleIndex == -1) // Angle is not specified yet: biopsy is off, or m_bSkipAngleSelection is on (which means set type but not angle)
    {
        // set default angle (if there are any valid angles) and turn biopsy guide on
        m_nAngleIndex = m_nNumAngles > 0 ? 0 : -1;
    }
    else
    {
        if ( params.getParamsSize() > 0 )
        {
            std::string sOption = params.getParamByIndex(0);
            bDeselect = ( sOption.compare( "deselect" ) == 0 );
        }

        if (bDeselect
            || (!m_bSkipAngleSelection && m_data.value))
        {
            // Deselecting the biopsy guide
            bTurningBiopsyGuideOff = true;
        }
    }

    syncInternalDataToULSIAndPercuNav(bTurningBiopsyGuideOff);

    const CTBArbitrationName  tbAttribName    =   appuiTBAnnot::BIOPSY_POSITION;
    const CEMInternalEventData1<CTBArbitrationName> acData( tbAttribName );

    if(m_data.value || bDeselect) // biopsy changed from on to off.
    {
        CommMgr::Instance().GetEventIf()->sendInternalEvent( CEMInternalEventCode("EVTMGR_INTERNAL_PERFORM_ACTION"), 
                                                        CActionId("EVTMGR_ACTION_TB_ARBITRATE_SUB"), acData ); 
        CStateMgr::Instance().setUpdateTSFlag(true);
        CommMgr::Instance().GetUIDMIf()->updateTSViews(true);
    }
    else
    {
        CommMgr::Instance().GetEventIf()->sendInternalEvent(CEMInternalEventCode( "EVTMGR_INTERNAL_BIOPSY_ACTIVE"), static_cast<int>(0), static_cast<int>(0) );
        CommMgr::Instance().GetEventIf()->sendInternalEvent( CEMInternalEventCode("EVTMGR_INTERNAL_PERFORM_ACTION"), 
                                                    CActionId("EVTMGR_ACTION_TB_ARBITRATE_ADD"), acData );

        if (m_bIsScanheadMotorized)
        {
            // Send out internal event to reset tilt angle
            CommMgr::Instance().GetEventIf()->sendInternalEvent("EVTMGR_INTERNAL_RESET_TILT", int(0), int(0));
        }
    }

    CommMgr::Instance().GetEventIf()->sendInternalEvent("EVTMGR_INTERNAL_XRES_TOGGLE_NOTIFY", (0), (0));
}

void evtmgr::CAc2DBiopsy::syncInternalDataToULSIAndPercuNav(bool bTurningBiopsyGuideOff)
{
    if (m_nBiopsyIsAllowed > 0) // don't call any biopsy guide calls if not supported
    {
        CMyUlsiMainScanheadClient ulsiMSClient;
        if (bTurningBiopsyGuideOff)
        {
            // Turn off biopsy guide
            ulsiMSClient->deselectBiopsyGuide().synchronize(ulsiMSClient.getTimeout());
        }
        else
        {
            // Setting guide type with or without angle selection
            CMyUlsiMainTransactionClient transClient;
            transClient->suspend(transClient.getTimeout());

            ulsiMSClient->selectBiopsyGuideType(m_selectedBiopsyType).synchronize(ulsiMSClient.getTimeout());
            if (!m_bSkipAngleSelection) {
                ulsiMSClient->selectBiopsyGuideAngle(m_nAngleIndex).synchronize(ulsiMSClient.getTimeout());
            }

            transClient->resume(transClient.getTimeout());
        }
    }
    const bool bDontCheckState = false; // IE make sure we only send out data if in a proc
    SendDataToPercuNav(bTurningBiopsyGuideOff, bDontCheckState);
}

//============================================================================
//
// Method which removes BIOPSY_POSITION  from the trackball list
// if Biopsy is enabled when Live loop capture is in progress
//============================================================================
void CAc2DBiopsy::updateTrackballListForOCI2D()
{
    CMyUlsiDataModeControlClient    usiDataClient;
    CMyAprMainProtocolClient aprClient;
    const CStateTree::CTreeName loopTreeName("BRANCH.Capture");
    CMyUlsiMainScanheadClient ulsiMainScanheadClient;

    const bool bLiveImaging = usiDataClient->getAcquisitionPipelineFreezeState(usiDataClient.getTimeout());
    const bool isProtocolLoaded = aprClient->getIsProtocolLoaded();
    const bool isProtocolPaused = rids::CRidsSingleton::getSingleton().getProtocolProxy().isProtocolPaused();
    const CStateId captureStateId = CStateMgr::Instance().getCurrentLeafId(loopTreeName);
    const bool bBiopsyEnabled = ulsiMainScanheadClient->getBiopsyGuideState().getValue(ulsiMainScanheadClient.getTimeout());
   
    if (bLiveImaging  && captureStateId == "Capture.L" && bBiopsyEnabled &&
        (!isProtocolLoaded || isProtocolPaused))
    {
        CTBArbitrationMgr::singleton().subTBArbitration(appuiTBAnnot::BIOPSY_POSITION);
    }
}
                            
//============================================================================
//
// Input from the design review regarding re-factoring identified this routine
// as a candidate for refactoring as the biopsy angles are almost identical.
// Re-factoring was scoped to moving the determination of whether the button was
// to be displayed into the updateEpmData() as it was base only on EPM data. The
// calling of USI was moved to occur only if the button was to be displayed. 
//
// Refactoring deferred was changing the XML files rather than having the switch
// statement below.
//
//============================================================================
bool CAc2DBiopsy::getInitialValue(CTSCtrlAttributes& tsCtrlAttrib)
{
    bool isCtrlVisible = false;
    bool isCtrlActive = false;
    if (tsCtrlAttrib.getName() != L"Btn_2d_Biopsy")
    {
        // we have bound to multiple buttons, but only want to control the state of one
        return false;
    }

    getCtrlAvailability(isCtrlVisible, isCtrlActive);
    tsCtrlAttrib.setVisible(isCtrlVisible);
    tsCtrlAttrib.setLed(isCtrlActive);
    updateTrackballListForOCI2D();
    return true;
}

bool CAc2DBiopsy::getInitialValue(CCPCtrlAttributes& cpCtrlAttrib)
{
    bool isCtrlAvailable = false;
    bool isCtrlActive = false;
    getCtrlAvailability(isCtrlAvailable, isCtrlActive);
    if (isCtrlAvailable)
    {
        cpCtrlAttrib.setCtrlAvailable(AVAILABLE);
        if (isCtrlActive)
        {
            cpCtrlAttrib.setCtrlActive(ACTIVE);
        }
        else
        {
            cpCtrlAttrib.setCtrlActive(NOT_ACTIVE);
        }
    }
    else
    {
        cpCtrlAttrib.setCtrlAvailable(NOT_AVAILABLE);
    }
    updateTrackballListForOCI2D();
    return true;
}

bool CAc2DBiopsy::getCtrlAvailability(bool& isCtrlAvailable, bool& isCtrlActive)
{

    if (m_nBiopsyIsAllowed == 0)
    {
        isCtrlAvailable = false;
    }
    else if (m_nBiopsyIsAllowed == 1 || m_nBiopsyIsAllowed == 2)
    {
        CMyUlsiMainScanheadClient           ulsiMSClient;
        CMyUlsiDataModeControlClient        ulsiDMCClient;
        CMyAprMainAcquisitionClient aprAcquisitionClient;

        // get the current state of dual display
        const auto bIsDualOrQuadDisplay = aprAcquisitionClient->inDualOrQuadDisplay(aprAcquisitionClient.getTimeout());
        const bool bIsTwoDLive = ulsiDMCClient->isTwoDLive(ulsiDMCClient.getTimeout());
        const ESysImagingModeType nSelectedImageMode = static_cast<ESysImagingModeType>(ulsiDMCClient->getSelectedImagingMode(ulsiDMCClient.getTimeout()));

        // Disable Biopsy TS button, when a non-Sagittal plane is active for the 
        // Biplane scanhead [Ref: CR0050692]
        bool nonSagittalPlaneWithBiplaneSh = false;
        if (COmniProbe::isBiplaneProbe())
        {
            // Convert the seek angle to an integer to avoid the trouble of comparing doubles
            // Add 0.5 to seekAngleDegrees to make sure we don't have any rounding issues before
            // converting to an int.
            const int rotationAngle = COmniProbe::getSeekAngleDegrees();
            // Biopsy is allowed only with SAGITTAL plane of Biplane scanhead
            nonSagittalPlaneWithBiplaneSh = (static_cast<int>(FEC_BIPLANE_ROTATION_ANGLE_SAGITTAL) != rotationAngle);
        }

        // When in SYS_IMAGING_MODE_TYPE_MATRIX_BIPLANE_ECHO3D we should have a reduced width region and the
        // biopsy buttons should not be grayed out. The case for 
        // SYS_IMAGING_MODE_TYPE_MATRIX_BIPLANE_ECHO3D handles the biplane reduced width region case.

        switch (nSelectedImageMode)
        {
        case SYS_IMAGING_MODE_TYPE_CF2D:
        case SYS_IMAGING_MODE_TYPE_CPA2D:
        case SYS_IMAGING_MODE_TYPE_ECHO_2D:
        case SYS_IMAGING_MODE_TYPE_SONOCT:
        case SYS_IMAGING_MODE_TYPE_CONTRAST_OCI2D:
        case SYS_IMAGING_MODE_TYPE_ELASTO2D:
        case SYS_IMAGING_MODE_TYPE_SHEARWAVE_IMAGING:
        case SYS_IMAGING_MODE_TYPE_SHEARWAVE_PQ:
        case SYS_IMAGING_MODE_TYPE_ECHO_2D_NEEDLE:
        case SYS_IMAGING_MODE_TYPE_SONOCT_NEEDLE:
        case SYS_IMAGING_MODE_TYPE_ATTEN_IMAGING:
        case SYS_IMAGING_MODE_TYPE_HRIQ:
        {
            if (bIsTwoDLive)
            {

                isCtrlAvailable = true;
                if (bIsDualOrQuadDisplay || nonSagittalPlaneWithBiplaneSh)
                {
                    isCtrlActive = false;
                    isCtrlAvailable = false;
                }
                else
                {
                    isCtrlAvailable = true; // un-gray 
                    pitk::CPitkFutureClient<bool> ulsiCtrl(ulsiMSClient->getNeedlePathDisplayed());
                    const bool bBiopsyDisplayed = ulsiCtrl.getValue(ulsiMSClient.getTimeout());
                    isCtrlActive = bBiopsyDisplayed;
                }
            }
            else
            {
                isCtrlAvailable = false;
            }
        }
        break;
        case SYS_IMAGING_MODE_TYPE_MATRIX_BIPLANE_ECHO3D:
        case SYS_IMAGING_MODE_TYPE_MATRIX_BIPLANE_CF3D:
        case SYS_IMAGING_MODE_TYPE_MATRIX_BIPLANE_CPA3D:
        case SYS_IMAGING_MODE_TYPE_MATRIX_BIPLANE_SONOCT3D:
        {
            if (bIsTwoDLive)
            {
                isCtrlAvailable = true;
                if (bIsDualOrQuadDisplay)
                {
                    isCtrlActive = false;
                    isCtrlAvailable = false;
                }
                else
                {
                    isCtrlAvailable = true; // un-gray 
                    pitk::CPitkFutureClient<bool> ulsiCtrl(ulsiMSClient->getNeedlePathDisplayed());
                    const bool bBiopsyDisplayed = ulsiCtrl.getValue(ulsiMSClient.getTimeout());
                    isCtrlActive = bBiopsyDisplayed;
                }
            }
            else
            {
                isCtrlAvailable = false;
            }
        }
        break;

        case SYS_IMAGING_MODE_TYPE_BLACK_LEVEL_3:
        case SYS_IMAGING_MODE_TYPE_BLACK_LEVEL_4:
        case SYS_IMAGING_MODE_TYPE_CF2D_CFMMODE:
        case SYS_IMAGING_MODE_TYPE_CF2D_CW:
        case SYS_IMAGING_MODE_TYPE_CF2D_MMODE:
        case SYS_IMAGING_MODE_TYPE_CF2D_PW:
        case SYS_IMAGING_MODE_TYPE_COMPOSITE_REPLAY:
        case SYS_IMAGING_MODE_TYPE_CONTRAST_ECHO_2D_HI:
        case SYS_IMAGING_MODE_TYPE_CONTRAST_ECHO_2D_LVO:
        case SYS_IMAGING_MODE_TYPE_CONTRAST_ECHO_2D_RT:
        case SYS_IMAGING_MODE_TYPE_CONTRAST_HPA2D_HI:
        case SYS_IMAGING_MODE_TYPE_CONTRAST_HPA2D_RT:
        case SYS_IMAGING_MODE_TYPE_CPA_XFOV:
        case SYS_IMAGING_MODE_TYPE_CPA2D_CW:
        case SYS_IMAGING_MODE_TYPE_CPA2D_PW:
        case SYS_IMAGING_MODE_TYPE_ECHO_3D_XFOV:
        case SYS_IMAGING_MODE_TYPE_ECHO_2D_CW:
        case SYS_IMAGING_MODE_TYPE_ECHO_2D_MMODE:
        case SYS_IMAGING_MODE_TYPE_ECHO_2D_PW:
        case SYS_IMAGING_MODE_TYPE_ECHO_2D_XFOV:
        case SYS_IMAGING_MODE_TYPE_EXPERIMENTAL:
        case SYS_IMAGING_MODE_TYPE_FREEHAND_CF3D:
        case SYS_IMAGING_MODE_TYPE_FREEHAND_CPA3D:
        case SYS_IMAGING_MODE_TYPE_FREEHAND_ECHO3D:
        case SYS_IMAGING_MODE_TYPE_FREEHAND_SONOCT3D:
        case SYS_IMAGING_MODE_TYPE_INVALID_IMAGING_MODE:
        case SYS_IMAGING_MODE_TYPE_MATRIX_BIPLANE_CF_CW:
        case SYS_IMAGING_MODE_TYPE_MATRIX_BIPLANE_CF_PW:
        case SYS_IMAGING_MODE_TYPE_MATRIX_BIPLANE_CPA_PW:
        case SYS_IMAGING_MODE_TYPE_MATRIX_BIPLANE_ECHO_CW:
        case SYS_IMAGING_MODE_TYPE_MATRIX_BIPLANE_ECHO_PW:
        case SYS_IMAGING_MODE_TYPE_MATRIX_BIPLANE_SONOCT_PW:
        case SYS_IMAGING_MODE_TYPE_MATRIX_CF3D:
        case SYS_IMAGING_MODE_TYPE_MATRIX_CPA3D:
        case SYS_IMAGING_MODE_TYPE_MATRIX_ECHO3D:
        case SYS_IMAGING_MODE_TYPE_MATRIX_FULL_VOLUME_CF3D:
        case SYS_IMAGING_MODE_TYPE_MATRIX_FULL_VOLUME_ECHO3D:
        case SYS_IMAGING_MODE_TYPE_MATRIX_LIVE_ECHO3D:
        case SYS_IMAGING_MODE_TYPE_MATRIX_SONOCT3D:
        case SYS_IMAGING_MODE_TYPE_MATRIX_SONOCT3D_XFOV:
        case SYS_IMAGING_MODE_TYPE_MOTORIZED_CF3D:
        case SYS_IMAGING_MODE_TYPE_MOTORIZED_CPA3D:
        case SYS_IMAGING_MODE_TYPE_MOTORIZED_ECHO3D:
        case SYS_IMAGING_MODE_TYPE_MOTORIZED_SONOCT3D:
        case SYS_IMAGING_MODE_TYPE_SONOCT_CF2D:
        case SYS_IMAGING_MODE_TYPE_SONOCT_CF2D_CW:
        case SYS_IMAGING_MODE_TYPE_SONOCT_CF2D_PW:
        case SYS_IMAGING_MODE_TYPE_SONOCT_CPA2D:
        case SYS_IMAGING_MODE_TYPE_SONOCT_CPA2D_CW:
        case SYS_IMAGING_MODE_TYPE_SONOCT_CPA2D_PW:
        case SYS_IMAGING_MODE_TYPE_SONOCT_MMODE:
        case SYS_IMAGING_MODE_TYPE_SONOCT_PW:
        case SYS_IMAGING_MODE_TYPE_SONOCT_XFOV:
        case SYS_IMAGING_MODE_TYPE_STATIC_CW:
        case SYS_IMAGING_MODE_TYPE_STATIC_PW:
        case SYS_IMAGING_MODE_TYPE_TDI2D:
        case SYS_IMAGING_MODE_TYPE_TDI2D_TISSUE_MM:
        case SYS_IMAGING_MODE_TYPE_TDI2D_TISSUE_PW:
        default: //not 1.L, 1.2L, 1.3L, xPlane Echo, xPlane Color
        {
            isCtrlAvailable = false;
        }
        break;
        }// end switch
    }
    else
    {
        // System does not yet support more than one biopsy guide.
      
        uiLogNoEHAandThrowAnticipatedFault(APPUI_ERROR_LOC(L"CAc2DBiopsy::getCtrlAvailability()", 1), eha::APPUI_FLT_EPM_DATA_PROBLEM, L"EPM Data: EPM_CMN_BIOPSY_BIOPSY1NUMGUIDESALLOWED has problem");
    }
    return true;
}

void CAc2DBiopsy::SendDataToPercuNav(bool bTurningBiopsyGuideOff, bool bDontCheckState)
{
    // If PercuNav is running, we inform it of the change in biopsy guide status.
    // if bDontCheckState == true then only check if PN or AI breast is licensed (ie ABARIS.exe is running) instead of in an active/paused procedure
    if (CAcPercuNavBase::isPercuNavActiveOrPaused() || CAcPercuNavBase::isAIBreastActiveOrPaused()
        || (bDontCheckState 
            && (CAcPercuNavBase::IsPercuNavLicensedOptionOn() || CAcPercuNavBase::IsAIBreastLicensedOptionOn()) ) )
    {
        bool bBiopsyGuideOn;
        if (bTurningBiopsyGuideOff)
        {
            bBiopsyGuideOn = false;
        }
        else
        {
            CMyUlsiMainScanheadClient ulsiMSClient;
            pitk::CPitkFutureClient<bool> ulsiCtrl(ulsiMSClient->getBiopsyGuideState());
            bBiopsyGuideOn = ulsiCtrl.getValue(ulsiMSClient.getTimeout());
        }
        const unsigned int nScanHeadPort = static_cast<unsigned int>(CScanHeadMgr::Instance().getSelectedPortId());
        EScanheadShId eScanhedID = static_cast<EScanheadShId>(0);
        if (nScanHeadPort != 0)
        {
            CMyUlsiMainScanheadClient usiMainShClient;
            eScanhedID = static_cast<EScanheadShId>(usiMainShClient->connectedScanhead(nScanHeadPort).getValue(usiMainShClient.getTimeout()));
        }
        CPercuNavMainControlClient().setBiopsyGuideState(eScanhedID, bBiopsyGuideOn, static_cast<EBiopsyType>(m_selectedBiopsyType), m_nAngleIndex);
    }
}
END_NS_EVTMGR
