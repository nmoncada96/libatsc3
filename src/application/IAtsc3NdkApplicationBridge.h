/*
 * IAtsc3NdkApplicationBridge.h
 *
 *  Created on: Aug 10, 2020
 *      Author: jjustman
 */
#include <string>

#ifndef SRC_APPLICATION_IATSC3NDKAPPLICATIONBRIDGE_H_
#define SRC_APPLICATION_IATSC3NDKAPPLICATIONBRIDGE_H_
using namespace std;

class IAtsc3NdkApplicationBridge {

    public:
        /* phy callback method(s)
         * TODO: for Saankhya use-case
         *
            atsc3NdkClient *me = (atsc3NdkClient *)ullUser; // same as &api
            return me->RxCallbackJJ(pData);
         */
       // virtual int atsc3_rx_callback_f(void*, uint64_t ullUser) = 0;

        virtual void LogMsg(const char *msg) = 0;
        virtual void LogMsg(const std::string &msg) = 0;
        virtual void LogMsgF(const char *fmt, ...) = 0;

        /** atsc3 service methods **/
        virtual int atsc3_slt_selectService(int service_id) = 0;
        virtual void atsc3_onMfuPacket(uint16_t packet_id, uint32_t mpu_sequence_number, uint32_t sample_number, uint8_t* buffer, uint32_t bufferLen, uint64_t presentationUs, uint32_t mfu_fragment_count_expected) = 0;
        virtual void atsc3_onMfuPacketCorrupt(uint16_t packet_id, uint32_t mpu_sequence_number, uint32_t sample_number, uint8_t* buffer, uint32_t bufferLen, uint64_t presentationUs, uint32_t mfu_fragment_count_expected, uint32_t mfu_fragment_count_rebuilt) = 0;
        virtual void atsc3_onMfuPacketCorruptMmthSampleHeader(uint16_t packet_id, uint32_t mpu_sequence_number, uint32_t sample_number, uint8_t* buffer, uint32_t bufferLen, uint64_t presentationUs, uint32_t mfu_fragment_count_expected, uint32_t mfu_fragment_count_rebuilt) = 0;
        virtual void atsc3_onInitHEVC_NAL_Extracted(uint16_t packet_id, uint32_t mpu_sequence_number, uint8_t* buffer, uint32_t bufferLen) = 0;

        virtual void atsc3_signallingContext_notify_video_packet_id_and_mpu_timestamp_descriptor(uint16_t video_packet_id, uint32_t mpu_sequence_number, uint64_t mpu_presentation_time_ntp64, uint32_t mpu_presentation_time_seconds, uint32_t mpu_presentation_time_microseconds) = 0;
        virtual void atsc3_signallingContext_notify_audio_packet_id_and_mpu_timestamp_descriptor(uint16_t audio_packet_id, uint32_t mpu_sequence_number, uint64_t mpu_presentation_time_ntp64, uint32_t mpu_presentation_time_seconds, uint32_t mpu_presentation_time_microsecond) = 0;
        virtual void atsc3_signallingContext_notify_stpp_packet_id_and_mpu_timestamp_descriptor(uint16_t stpp_packet_id, uint32_t mpu_sequence_number, uint64_t mpu_presentation_time_ntp64, uint32_t mpu_presentation_time_seconds, uint32_t mpu_presentation_time_microseconds) = 0;
        virtual void atsc3_lls_sls_alc_on_package_extract_completed_callback_jni(atsc3_route_package_extracted_envelope_metadata_and_payload_t* atsc3_route_package_extracted_envelope_metadata_and_payload_t) = 0;

        virtual void atsc3_onExtractedSampleDuration(uint16_t packet_id, uint32_t mpu_sequence_number,
                                                     uint32_t extracted_sample_duration_us) = 0;

        virtual void atsc3_setVideoWidthHeightFromTrak(uint32_t width, uint32_t height) = 0;

        virtual int atsc3_slt_alc_select_additional_service(int service_id) = 0;

        virtual int atsc3_slt_alc_clear_additional_service_selections() = 0;

        virtual vector<string>
        atsc3_slt_alc_get_sls_metadata_fragments_content_locations_from_monitor_service_id(int service_id, const char* to_match_content_type) = 0;

        virtual vector<string>
        atsc3_slt_alc_get_sls_route_s_tsid_fdt_file_content_locations_from_monitor_service_id(int service_id) = 0;

        virtual void atsc3_lls_sls_alc_on_object_close_flag_s_tsid_content_location_jni(uint32_t tsi, uint32_t toi,
                                                                                        char *content_location) = 0;

        virtual void atsc3_lls_sls_alc_on_route_mpd_patched_jni(uint16_t service_id) = 0;

        virtual void atsc3_onSlsTablePresent(const char *sls_payload_xml) = 0;

        virtual void atsc3_onAlcObjectStatusMessage(const char *fmt, ...) = 0;

        virtual void atsc3_sls_on_held_trigger_received_callback_jni(uint16_t service_id, const char *held_payload) = 0;

        virtual void atsc3_onMfuSampleMissing(uint16_t i, uint32_t i1, uint32_t i2) = 0;

        //virtual void set_plp_settings(jint *a_plp_ids, jsize sa_plp_size) = 0;

        virtual string get_android_temp_folder() = 0;

        virtual int pinFromRxProcessingThread() = 0;

        virtual int RxThread() = 0;
};



#endif /* SRC_APPLICATION_IATSC3NDKAPPLICATIONBRIDGE_H_ */
