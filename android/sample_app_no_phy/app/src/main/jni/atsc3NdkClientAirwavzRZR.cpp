//
// Created by Jason Justman on 2019-11-23.
//

#include <atsc3_utils.h>
#include <atsc3_phy_mmt_player_bridge.h>
#include "atsc3NdkClient.h"

#include "atsc3NdkClientAirwavzRZR.h"

#include "props_api.h"
#include "redzone_c_api.h"

#include "basebandparser.h"
#include "alpparser.h"

extern atsc3NdkClientAirwavzRZR apiImpl;

bool atsc3NdkClientAirwavzRZR::captureThreadShouldRun = false;
bool atsc3NdkClientAirwavzRZR::processThreadShouldRun = false;
bool atsc3NdkClientAirwavzRZR::tunerStatusThreadShouldRun = false;
bool atsc3NdkClientAirwavzRZR::tunerStatusThreadShouldPollTunerStatus = false;

RedZoneCaptureHandle_t hRedZoneCapture;

atsc3NdkClient* atsc3NdkClientAirwavzRZR::atsc3NdkClient_ref = NULL;

//NDK JNI main dispatcher reference
void atsc3NdkClientAirwavzRZR::Init(atsc3NdkClient* ref_) {
    atsc3NdkClientAirwavzRZR::atsc3NdkClient_ref = ref_;

}

int atsc3NdkClientAirwavzRZR::Open(int fd_, int bus_, int addr_) {
    fd = fd_;
    bus = bus_;
    addr = addr_;
    int     rval;
    int     verbosity = 9;      // suggest a value of 2 (0 - 9 legal) to set debug output level;
    
    printf( "atsc3NdkClientAirwavzRZR Entered\n");

	if (RedZoneCaptureOpen(&hRedZoneCapture)) {
		printf("RedZoneCaptureOpen: Failed to initialize RedZoneCapture status = %d\n", rval);
		return 1;

	}
    rval = RedZoneCaptureSetProp(hRedZoneCapture, RedZoneLoggingVerboseMode, &verbosity, sizeof(verbosity));
    printf("RedZoneCaptureSetProp: RedZoneLoggingVerboseMode returned %d\n", rval);

    rval = RedZoneCaptureInitSysDevice(hRedZoneCapture, fd );
    if (rval != RZR_SUCCESS)
    {
        printf("RedZoneCaptureInitSysDevice: Failed to initialize RedZoneCapture %d\nThis could be a result on a firmware download and re-ennumeration - try again\n", rval);
        return 1;
    }

    RedZoneOperatingMode opmode = OperatingModeATSC3;
    rval = RedZoneCaptureSetProp(hRedZoneCapture, RedZoneOperatingModeProp, &opmode, sizeof(opmode));
    printf("RedZoneCaptureSetProp: RedZoneOperatingModePropMode returned %d\n", rval);

    printf( "atsc3NdkClientAirwavzRZR:  success\n");
    return 0;
}

int atsc3NdkClientAirwavzRZR::Tune(int freqKhz, int plpId) {

    vector<int> myPlps;
    myPlps.push_back(plpId);

    return TuneMultiplePLP(freqKhz, myPlps);
}

/**
 * jjustman-2019-12-17: TODO - refactor this to InitTunerAndDemodTransfer
 * @param freqKhz
 * @param plpIds
 * @return
 */
int atsc3NdkClientAirwavzRZR::TuneMultiplePLP(int freqKhz, vector<int> plpIds) {

    int rval;

    int cThread_ret = 0;
    int pThread_ret = 0;
    int sThread_ret = 0;


    //1. config plp's
    //2. set tuner frequency
    //3. start processing threads

    printf( "atsc3NdkClientAirwavzRZR::TuneMultiplePLP freq = %d\n", freqKhz);

    rval = RedZoneCaptureSetProp(hRedZoneCapture, RedZoneFrequencyKHzProp, &freqKhz, sizeof(freqKhz));
    printf("tune returned %d\n", rval);

    RedZonePLPSet plpset = {255, 255, 255, 255};

    int nplp = plpIds.size();

    if (nplp > 4) {
        printf("Request %d PLPs only the first 4 will be tuned\n", nplp);
        nplp = 4;
    }
    switch (nplp) {

        case 0:
            printf("No PLPs specified\n");
            break;
        case 4:
            plpset.plp3_id = plpIds[3];
            /* fall through */
        case 3:
            plpset.plp2_id = plpIds[2];
            /* fall through */
        case 2:
            plpset.plp1_id = plpIds[1];
            /* fall through */
        case 1:
            plpset.plp0_id = plpIds[0];
            /* fall through */
            break;
    }
    if (nplp >0) {
        rval = RedZoneCaptureSetProp(hRedZoneCapture, RedZonePLPSelectionProp, &plpset, sizeof(plpset));
        printf("Set PLP Selection returned %d\n", rval);
    }
    atsc3NdkClientAirwavzRZR::captureThreadShouldRun = true;


    //start our Capture thread which will invoke libusb_submit_transfer and libusb_handle_events
    if (!cThreadID) {

//        if (!atsc3_sl_tlv_block) {
//            atsc3_sl_tlv_block = block_Alloc(BUFFER_SIZE);
//        }
//
//        printf("creating capture thread, cb buffer size: %d, tlv_block_size: %d",
//               CB_SIZE, BUFFER_SIZE);

        cThread_ret = pthread_create(&cThreadID, NULL, (THREADFUNCPTR) &atsc3NdkClientAirwavzRZR::CaptureThread, NULL);
        printf("created CaptureThread, cThreadID is: %d", (int)cThreadID);
        if (cThread_ret != 0) {
            atsc3NdkClientAirwavzRZR::captureThreadShouldRun = false;
            printf("Capture Thread launched unsuccessfully, cThread_ret: %d", cThread_ret);
            return 0;
        }
    } else {
        printf("using existing CaptureThread");
    }

#if 0
    //create our ProcessThread - handles enqueued TLV payload from CaptureThread
    if (!pThreadID) {

        atsc3NdkClientAirwavzRZR::processThreadShouldRun = true;

        pThread_ret = pthread_create(&pThreadID, NULL, (THREADFUNCPTR) &atsc3NdkClientAirwavzRZR::ProcessThread, NULL);
        if (pThread_ret != 0) {
            atsc3NdkClientAirwavzRZR::processThreadShouldRun = 0;
            printf("Process Thread launched unsuccessfully, ret: %d", pThread_ret);
            return -1;
        }
    } else {
        printf("using existing ProcessThread, pThread is: %d", (int)pThreadID);
    }
#endif

    if (!tsThreadID) {

        atsc3NdkClientAirwavzRZR::tunerStatusThreadShouldRun = true;

        sThread_ret = pthread_create(&tsThreadID, NULL, (THREADFUNCPTR) &atsc3NdkClientAirwavzRZR::TunerStatusThread, NULL);
        if (sThread_ret != 0) {
            atsc3NdkClientAirwavzRZR::tunerStatusThreadShouldRun = false;
            printf("Status Thread launched unsuccessfully, ret: %d", sThread_ret);
            return 0;
        }
    } else {
        printf("using existing TunerStatusThread, sThread is: %d", (int)tsThreadID);
    }

    atsc3NdkClientAirwavzRZR::atsc3NdkClient_ref->LogMsgF("TuneMultiplePLP: completed: capture: %ul, process: %ul, tunerStatus: %ul", cThreadID, pThreadID, tsThreadID);

    return 0;
}



void atsc3NdkClientAirwavzRZR::processTLVFromCallback()
{
//    int bytesRead = CircularBufferPop(atsc3NdkClientAirwavzRZR::cb, BUFFER_SIZE, (char*)&processDataCircularBufferForCallback);
//    if(!atsc3_sl_tlv_block) {
//        printf("ERROR: atsc3NdkClientSlImpl::processTLVFromCallback - atsc3_sl_tlv_block is NULL!");
//        return;
//    }
//
//    if(bytesRead) {
//
//        block_Write(atsc3_sl_tlv_block, (uint8_t*)&processDataCircularBufferForCallback, bytesRead);
//        block_Rewind(atsc3_sl_tlv_block);
//
//        bool atsc3_sl_tlv_payload_complete = false;
//
//        do {
//            atsc3_sl_tlv_payload = atsc3_sl_tlv_payload_parse_from_block_t(atsc3_sl_tlv_block);
//
//            if(atsc3_sl_tlv_payload) {
//                atsc3_sl_tlv_payload_dump(atsc3_sl_tlv_payload);
//                if(atsc3_sl_tlv_payload->alp_payload_complete) {
//                    atsc3_sl_tlv_payload_complete = true;
//
//                    block_Rewind(atsc3_sl_tlv_payload->alp_payload);
//                    atsc3_alp_packet_t* atsc3_alp_packet = atsc3_alp_packet_parse(atsc3_sl_tlv_payload->alp_payload);
//                    if(atsc3_alp_packet) {
//                        alp_completed_packets_parsed++;
//
//                        alp_total_bytes += atsc3_alp_packet->alp_payload->p_size;
//
//                        if(atsc3_alp_packet->alp_packet_header.packet_type == 0x00) {
//
//                            block_Rewind(atsc3_alp_packet->alp_payload);
//                            atsc3_phy_mmt_player_bridge_process_packet_phy(atsc3_alp_packet->alp_payload);
//
//                        } else if(atsc3_alp_packet->alp_packet_header.packet_type == 0x4) {
//                            alp_total_LMTs_recv++;
//                        }
//
//                        atsc3_alp_packet_free(&atsc3_alp_packet);
//                    }
//                    //free our atsc3_sl_tlv_payload
//                    atsc3_sl_tlv_payload_free(&atsc3_sl_tlv_payload);
//
//                } else {
//                    atsc3_sl_tlv_payload_complete = false;
//                    //jjustman-2019-12-29 - noisy, TODO: wrap in __DEBUG macro check
//                    //printf("alp_payload->alp_payload_complete == FALSE at pos: %d, size: %d", atsc3_sl_tlv_block->i_pos, atsc3_sl_tlv_block->p_size);
//                }
//            } else {
//                atsc3_sl_tlv_payload_complete = false;
//                //jjustman-2019-12-29 - noisy, TODO: wrap in __DEBUG macro check
//                //printf("ERROR: alp_payload IS NULL, short TLV read?  at atsc3_sl_tlv_block: i_pos: %d, p_size: %d", atsc3_sl_tlv_block->i_pos, atsc3_sl_tlv_block->p_size);
//            }
//
//        } while(atsc3_sl_tlv_payload_complete);
//
//
//        if(atsc3_sl_tlv_payload && !atsc3_sl_tlv_payload->alp_payload_complete && atsc3_sl_tlv_block->i_pos > atsc3_sl_tlv_payload->sl_tlv_total_parsed_payload_size) {
//            //accumulate from our last starting point and continue iterating during next bbp
//            atsc3_sl_tlv_block->i_pos -= atsc3_sl_tlv_payload->sl_tlv_total_parsed_payload_size;
//            //free our atsc3_sl_tlv_payload
//            atsc3_sl_tlv_payload_free(&atsc3_sl_tlv_payload);
//        }
//
//        if(atsc3_sl_tlv_block->p_size > atsc3_sl_tlv_block->i_pos) {
//            //truncate our current block_t starting at i_pos, and then read next i/o block
//            block_t* temp_sl_tlv_block = block_Duplicate_from_position(atsc3_sl_tlv_block);
//            block_Destroy(&atsc3_sl_tlv_block);
//            atsc3_sl_tlv_block = temp_sl_tlv_block;
//            block_Seek(atsc3_sl_tlv_block, atsc3_sl_tlv_block->p_size);
//        } else if(atsc3_sl_tlv_block->p_size == atsc3_sl_tlv_block->i_pos) {
//            //clear out our tlv block as we are the "exact" size of our last alp packet
//
//            block_Destroy(&atsc3_sl_tlv_block);
//            atsc3_sl_tlv_block = block_Alloc(BUFFER_SIZE);
//        } else {
//            printf("atsc3_sl_tlv_block: positioning mismatch: i_pos: %d, p_size: %d - rewinding and seeking for magic packet?", atsc3_sl_tlv_block->i_pos, atsc3_sl_tlv_block->p_size);
//
//            //jjustman: 2019-11-23: rewind in order to try seek for our magic packet in the next loop here
//            block_Rewind(atsc3_sl_tlv_block);
//        }
//    }
}


void atsc3NdkClientAirwavzRZR::RxDataCallback(unsigned char *data, long len)
{
    //printf("atsc3NdkClientAirwavzRZR::RxDataCallback: pushing data: %p, len: %d", data, len);
    //CircularBufferPush(atsc3NdkClientSlImpl::cb, (char *)data, len);
}

void* atsc3NdkClientAirwavzRZR::CaptureThread(void*)
{
    atsc3NdkClientAirwavzRZR::atsc3NdkClient_ref->pinFromRxCaptureThread();

    //...((RxDataCB)&atsc3NdkClientAirwavzRZR::RxDataCallback);
    return 0;
}


//Process Thread Impl
void* atsc3NdkClientAirwavzRZR::ProcessThread(void*)
{
    apiImpl.resetProcessThreadStatistics();

    atsc3NdkClientAirwavzRZR::atsc3NdkClient_ref->pinFromRxProcessingThread();

    while (atsc3NdkClientAirwavzRZR::processThreadShouldRun)
    {
        //printf("atsc3NdkClientSlImpl::ProcessThread: getDataSize is: %d", CircularBufferGetDataSize(cb));

//        if (CircularBufferGetDataSize(cb) >= BUFFER_SIZE) {
//            apiImpl.processTLVFromCallback();
//        } else {
//            usleep(10000);
//        }
    }

    return 0;
}

void* atsc3NdkClientAirwavzRZR::TunerStatusThread(void*)
{
    int rval;

    atsc3NdkClientAirwavzRZR::atsc3NdkClient_ref->pinFromRxStatusThread();


    while(atsc3NdkClientAirwavzRZR::tunerStatusThreadShouldRun) {

//        //only actively poll the tuner status if the RF status window is visible
//        if(!atsc3NdkClientAirwavzRZR::tunerStatusThreadShouldPollTunerStatus) {
            usleep(2000000);
//            continue;
//        }
//
//
//
//        /*jjustman-2020-01-06: For the SL3000/SiTune, we will have 3 status attributes with the following possible values:
//
//                tunerInfo.status:   SL_TUNER_STATUS_NOT_LOCKED (0)
//                                    SL_TUNER_STATUS_LOCKED (1)
//
//                demodLockStatus:    SL_DEMOD_STATUS_NOT_LOCK (0)
//                                    SL_DEMOD_STATUS_LOCK (1)
//
//                cpuStatus:          (cpuStatus == 0xFFFFFFFF) ? "RUNNING" : "HALTED",
//         */
//
//
//
//        snr = (float)perfDiag.GlobalSnrLinearScale / 16384;
//        snr = 10000.0 * log10(snr); //10

          unsigned char lock;
          int   SNR, RSSi;

          rval = RedZoneCaptureGetProp( hRedZoneCapture, RedZoneMasterLockProp, &lock, sizeof(lock));
          printf("get Lock %d status %d\n", lock, rval);
          rval = RedZoneCaptureGetProp( hRedZoneCapture, RedZoneSNRProp, &SNR, sizeof(SNR));
          printf("get SNR %d status %d\n", SNR, rval);
          rval = RedZoneCaptureGetProp( hRedZoneCapture, RedZoneRSSIProp, &RSSi, sizeof(RSSi));
          printf("get RSSi %d status %d\n", RSSi, rval);


//
//        ber_l1b = (float)perfDiag.NumBitErrL1b / perfDiag.NumFecBitsL1b; // //aBerPreLdpcE7,
//        ber_l1d = (float)perfDiag.NumBitErrL1d / perfDiag.NumFecBitsL1d;//aBerPreBchE9,
//        ber_plp0 = (float)perfDiag.NumBitErrPlp0 / perfDiag.NumFecBitsPlp0; //aFerPostBchE6,
//
//        printf("atsc3NdkClientAirwavzRZR::TunerStatusThread: tunerInfo.status: %d, tunerInfo.signalStrength: %f, cpuStatus: %s, demodLockStatus: %d, globalSnr: %f",
//                tunerInfo.status,
//                tunerInfo.signalStrength,
//                (cpuStatus == 0xFFFFFFFF) ? "RUNNING" : "HALTED",
//                demodLockStatus,
//                perfDiag.GlobalSnrLinearScale);
//
//
        //atsc3NdkClientSLRef->atsc3_update_rf_stats(
        atsc3NdkClient_ref->atsc3_update_rf_stats(
                1,                  // tunerInfo.status == 1,
                RSSi * 10,          // tunerInfo.signalStrength,
                0,                  // modcod_valid
                0,                  // plp fec type
                0,                  // plp mod
                0,                  // plp cod
                RSSi * 10,          // RFLevel1000
                SNR * 10,           // SNR1000
                0,                  // ber pre ldpc
                0,                  // ber pre bch
                0,                  // fer post
                lock,               // demod lock
                1,                  // cpu status
                0,                  // plp lls?
                0);

//        atsc3NdkClientAirwavzRZR->atsc3_update_rf_bw_stats(apiImpl.alp_completed_packets_parsed, apiImpl.alp_total_bytes, apiImpl.alp_total_LMTs_recv);

    }
    return 0;
}


void atsc3NdkClientAirwavzRZR::resetProcessThreadStatistics() {
    alp_completed_packets_parsed = 0;
    alp_total_bytes = 0;
    alp_total_LMTs_recv = 0;
}
