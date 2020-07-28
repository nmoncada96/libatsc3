/*
 * atsc3_sls_alc_flow.c
 *
 *  Created on: Jul 28, 2020
 *      Author: jjustman
 */

#include "atsc3_sls_alc_flow.h"

int _SLS_ALC_FLOW_INFO_ENABLED = 0;
int _SLS_ALC_FLOW_DEBUG_ENABLED = 0;
int _SLS_ALC_FLOW_TRACE_ENABLED = 0;


ATSC3_VECTOR_BUILDER_TYPEDEF_STRUCT_METHODS_IMPLEMENTATION(atsc3_sls_alc_flow);

void atsc3_sls_alc_flow_typedef_free(atsc3_sls_alc_flow_t** atsc3_sls_alc_flow_p) {
	if(atsc3_sls_alc_flow_p) {
		atsc3_sls_alc_flow_t* atsc3_sls_alc_flow = *atsc3_sls_alc_flow_p;

		//jjustman-2020-07-28 - TODO - make sure we're not doublefree'ing
		if(atsc3_sls_alc_flow) {
			if(atsc3_sls_alc_flow->atsc3_route_s_tsid_RS_LS) {
				atsc3_route_s_tsid_RS_LS_free(&atsc3_sls_alc_flow->atsc3_route_s_tsid_RS_LS);
			} else {
				if(atsc3_sls_alc_flow->media_info) {
					atsc3_route_s_tsid_RS_LS_SrcFlow_ContentInfo_MediaInfo_free(&atsc3_sls_alc_flow->media_info);
				}
			}
			atsc3_sls_alc_flow_free_atsc3_route_object(atsc3_sls_alc_flow);

			free(atsc3_sls_alc_flow);
			atsc3_sls_alc_flow = NULL;
		}
		*atsc3_sls_alc_flow_p = NULL;
	}
}

/*
 * atsc3_sls_alc_flow_route_object_add_unique_lct_packet_received
 *
 * 	step 1. find our matching tsi/toi flow here
 */
atsc3_route_object_t* atsc3_sls_alc_flow_route_object_add_unique_lct_packet_received(atsc3_sls_alc_flow_v* atsc3_sls_alc_flow_vector, atsc3_alc_packet_t* atsc3_alc_packet) {
	atsc3_route_object_t* atsc3_route_object = NULL;

	//find or create our alc_flow
	atsc3_sls_alc_flow_t* atsc3_sls_alc_flow = atsc3_sls_alc_flow_find_or_create_entry_from_alc_packet(atsc3_sls_alc_flow_vector, atsc3_alc_packet);

	_ATSC3_SLS_ALC_FLOW_TRACE("atsc3_sls_alc_flow_route_object_add_unique_lct_packet_received before find_or_create_route_object, atsc3_sls_alc_flow_vector.count: %d, tsi: %d, toi: %d, atsc3_sls_alc_flow: %p, atsc3_route_object_v: %d ",
			atsc3_sls_alc_flow_vector->count,
			atsc3_alc_packet->def_lct_hdr->tsi, atsc3_alc_packet->def_lct_hdr->toi,
			atsc3_sls_alc_flow,
			atsc3_sls_alc_flow->atsc3_route_object_v.count);

	//find or create our route_object
	atsc3_route_object = atsc3_sls_alc_flow_find_or_create_route_object_from_alc_packet(atsc3_sls_alc_flow, atsc3_alc_packet);

	_ATSC3_SLS_ALC_FLOW_TRACE("atsc3_sls_alc_flow_route_object_add_unique_lct_packet_received after find_or_create_route_object, obj: %p, tsi: %d, toi: %d, atsc3_sls_alc_flow: %p, count: %d ",
			atsc3_route_object,
			atsc3_alc_packet->def_lct_hdr->tsi, atsc3_alc_packet->def_lct_hdr->toi,
			atsc3_sls_alc_flow, atsc3_sls_alc_flow->atsc3_route_object_v.count);

	atsc3_route_object_add_or_update_lct_packet_received(atsc3_route_object, atsc3_alc_packet);

	return atsc3_route_object;
}


atsc3_sls_alc_flow_t* atsc3_sls_alc_flow_find_or_create_entry_from_alc_packet(atsc3_sls_alc_flow_v* atsc3_sls_alc_flow_vector, atsc3_alc_packet_t* atsc3_alc_packet) {
	atsc3_sls_alc_flow_t* matching_atsc3_sls_alc_flow = atsc3_sls_alc_flow_find_entry_from_alc_packet(atsc3_sls_alc_flow_vector, atsc3_alc_packet);
	if(matching_atsc3_sls_alc_flow == NULL) {
		//create a new entry
		matching_atsc3_sls_alc_flow = atsc3_sls_alc_flow_new();
		matching_atsc3_sls_alc_flow->tsi = atsc3_alc_packet->def_lct_hdr->tsi;
		atsc3_sls_alc_flow_add(atsc3_sls_alc_flow_vector, matching_atsc3_sls_alc_flow);
	}

	return matching_atsc3_sls_alc_flow;
}


//&& to_check_atsc3_sls_alc_flow_nrt->toi == alc_packet->def_lct_hdr->toi
atsc3_sls_alc_flow_t* atsc3_sls_alc_flow_find_entry_from_alc_packet(atsc3_sls_alc_flow_v* atsc3_sls_alc_flow_vector, atsc3_alc_packet_t* alc_packet) {
	atsc3_sls_alc_flow_t* matching_atsc3_sls_alc_flow = NULL;
	atsc3_sls_alc_flow_t* to_check_atsc3_sls_alc_flow = NULL;

	for(int i=0; i < atsc3_sls_alc_flow_vector->count && !matching_atsc3_sls_alc_flow; i++) {
		to_check_atsc3_sls_alc_flow = atsc3_sls_alc_flow_vector->data[i];
		if(to_check_atsc3_sls_alc_flow->tsi == alc_packet->def_lct_hdr->tsi) {
			matching_atsc3_sls_alc_flow = to_check_atsc3_sls_alc_flow;
		}
	}

	return matching_atsc3_sls_alc_flow;
}

atsc3_route_object_t* atsc3_sls_alc_flow_find_or_create_route_object_from_alc_packet(atsc3_sls_alc_flow_t* atsc3_sls_alc_flow, atsc3_alc_packet_t* alc_packet) {
	atsc3_route_object_t* matching_atsc3_route_object = atsc3_sls_alc_flow_find_route_object_entry_from_alc_packet(atsc3_sls_alc_flow, alc_packet);
	if(matching_atsc3_route_object == NULL) {
		//create a new entry
		matching_atsc3_route_object = atsc3_route_object_new();
		atsc3_route_object_set_alc_flow_and_tsi_toi(matching_atsc3_route_object, atsc3_sls_alc_flow, alc_packet);
		//todo - impl methods
		atsc3_sls_alc_flow_add_atsc3_route_object(atsc3_sls_alc_flow, matching_atsc3_route_object);
	}

	return matching_atsc3_route_object;
}

atsc3_route_object_t* atsc3_sls_alc_flow_find_route_object_entry_from_alc_packet(atsc3_sls_alc_flow_t* atsc3_sls_alc_flow, atsc3_alc_packet_t* alc_packet) {
	atsc3_route_object_t* matching_atsc3_route_object = NULL;
	atsc3_route_object_t* to_check_atsc3_route_object = NULL;

	for(int i=0; i < atsc3_sls_alc_flow->atsc3_route_object_v.count && !matching_atsc3_route_object; i++) {
		to_check_atsc3_route_object = atsc3_sls_alc_flow->atsc3_route_object_v.data[i];
		if(to_check_atsc3_route_object->tsi == alc_packet->def_lct_hdr->tsi && to_check_atsc3_route_object->toi == alc_packet->def_lct_hdr->toi) {
			matching_atsc3_route_object = to_check_atsc3_route_object;
		}
	}

	return matching_atsc3_route_object;
}

//still needed!
//jjustman-2020-07-28
//for matching contentInfo.mediaInfo@repId
atsc3_sls_alc_flow_t* atsc3_sls_alc_flow_add_entry_unique_tsi(atsc3_sls_alc_flow_v* atsc3_sls_alc_flow, uint32_t tsi, atsc3_route_s_tsid_RS_LS_SrcFlow_ContentInfo_MediaInfo_t* media_info) {
	atsc3_sls_alc_flow_t* matching_atsc3_sls_alc_flow = atsc3_sls_alc_flow_find_entry_tsi(atsc3_sls_alc_flow, tsi);
	if(matching_atsc3_sls_alc_flow == NULL) {
		_ATSC3_SLS_ALC_FLOW_DEBUG("atsc3_sls_alc_flow_add_entry_unique_tsi: adding new entry to %p, tsi: %d\n", &atsc3_sls_alc_flow, tsi);
		//create a new entry
		matching_atsc3_sls_alc_flow = atsc3_sls_alc_flow_new();
		matching_atsc3_sls_alc_flow->tsi = tsi;

		if(media_info) {
			matching_atsc3_sls_alc_flow->media_info = atsc3_route_s_tsid_RS_LS_SrcFlow_ContentInfo_MediaInfo_clone(media_info);
			_ATSC3_SLS_ALC_FLOW_DEBUG("atsc3_sls_alc_flow_add_entry_unique_tsi_toi_init: cloned media_info is now: %p (stsid: %p), content_type: %s, lang: %s, rep_id: %s",
					matching_atsc3_sls_alc_flow->media_info,
					media_info,
					matching_atsc3_sls_alc_flow->media_info->content_type,
					matching_atsc3_sls_alc_flow->media_info->lang,
					matching_atsc3_sls_alc_flow->media_info->rep_id);

		} else {
			_ATSC3_SLS_ALC_FLOW_WARN("atsc3_sls_alc_flow_add_entry_unique_tsi_toi_init: media_info is NULL for new entry to %p, tsi: %d", &atsc3_sls_alc_flow, tsi);
		}

		atsc3_sls_alc_flow_add(atsc3_sls_alc_flow, matching_atsc3_sls_alc_flow);
	} else {
		//make sure we have our media_info
		if(media_info && !matching_atsc3_sls_alc_flow->media_info) {
			matching_atsc3_sls_alc_flow->media_info = atsc3_route_s_tsid_RS_LS_SrcFlow_ContentInfo_MediaInfo_clone(media_info);
			_ATSC3_SLS_ALC_FLOW_DEBUG("atsc3_sls_alc_flow_add_entry_unique_tsi_toi_init: cloned media_info is now: %p (stsid: %p), content_type: %s, lang: %s, rep_id: %s",
					matching_atsc3_sls_alc_flow->media_info,
					media_info,
					matching_atsc3_sls_alc_flow->media_info->content_type,
					matching_atsc3_sls_alc_flow->media_info->lang,
					matching_atsc3_sls_alc_flow->media_info->rep_id);

		}
	}

	return matching_atsc3_sls_alc_flow;
}


atsc3_sls_alc_flow_t* atsc3_sls_alc_flow_add_entry_unique_tsi_toi_init(atsc3_sls_alc_flow_v* atsc3_sls_alc_flow, uint32_t tsi, uint32_t toi_init, atsc3_route_s_tsid_RS_LS_SrcFlow_ContentInfo_MediaInfo_t* media_info) {
	atsc3_sls_alc_flow_t* matching_atsc3_sls_alc_flow = atsc3_sls_alc_flow_find_entry_tsi_toi_init(atsc3_sls_alc_flow, tsi, toi_init);
	if(matching_atsc3_sls_alc_flow == NULL) {
		_ATSC3_SLS_ALC_FLOW_DEBUG("atsc3_sls_alc_flow_add_entry_unique_tsi_toi_init: adding new entry to %p, tsi: %d, toi_init: %d\n", &atsc3_sls_alc_flow, tsi, toi_init);
		//create a new entry
		matching_atsc3_sls_alc_flow = atsc3_sls_alc_flow_new();
		matching_atsc3_sls_alc_flow->tsi = tsi;
		matching_atsc3_sls_alc_flow->toi_init = toi_init;

		if(media_info) {
			matching_atsc3_sls_alc_flow->media_info = atsc3_route_s_tsid_RS_LS_SrcFlow_ContentInfo_MediaInfo_clone(media_info);
			_ATSC3_SLS_ALC_FLOW_DEBUG("atsc3_sls_alc_flow_add_entry_unique_tsi_toi_init: cloned media_info is now: %p (stsid: %p), content_type: %s, lang: %s, rep_id: %s",
					matching_atsc3_sls_alc_flow->media_info,
					media_info,
					matching_atsc3_sls_alc_flow->media_info->content_type,
					matching_atsc3_sls_alc_flow->media_info->lang,
					matching_atsc3_sls_alc_flow->media_info->rep_id);

		} else {
			_ATSC3_SLS_ALC_FLOW_WARN("atsc3_sls_alc_flow_add_entry_unique_tsi_toi_init: media_info is NULL for new entry to %p, tsi: %d, toi_init: %d\n", &atsc3_sls_alc_flow, tsi, toi_init);
		}

		atsc3_sls_alc_flow_add(atsc3_sls_alc_flow, matching_atsc3_sls_alc_flow);
	}

	return matching_atsc3_sls_alc_flow;
}

atsc3_sls_alc_flow_t* atsc3_sls_alc_flow_find_entry_tsi_toi_init(atsc3_sls_alc_flow_v* atsc3_sls_alc_flow, uint32_t tsi, uint32_t toi_init) {
	atsc3_sls_alc_flow_t* matching_atsc3_sls_alc_flow = NULL;
	atsc3_sls_alc_flow_t* to_check_atsc3_sls_alc_flow = NULL;

	for(int i=0; i < atsc3_sls_alc_flow->count && !matching_atsc3_sls_alc_flow; i++) {
		to_check_atsc3_sls_alc_flow = atsc3_sls_alc_flow->data[i];
		if(to_check_atsc3_sls_alc_flow->tsi == tsi && to_check_atsc3_sls_alc_flow->toi_init == toi_init) {
			matching_atsc3_sls_alc_flow = to_check_atsc3_sls_alc_flow;
		}
	}

	return matching_atsc3_sls_alc_flow;
}


atsc3_sls_alc_flow_t* atsc3_sls_alc_flow_find_entry_tsi(atsc3_sls_alc_flow_v* atsc3_sls_alc_flow, uint32_t tsi) {
	atsc3_sls_alc_flow_t* matching_atsc3_sls_alc_flow = NULL;
	atsc3_sls_alc_flow_t* to_check_atsc3_sls_alc_flow = NULL;

	for(int i=0; i < atsc3_sls_alc_flow->count && !matching_atsc3_sls_alc_flow; i++) {
		to_check_atsc3_sls_alc_flow = atsc3_sls_alc_flow->data[i];
		if(to_check_atsc3_sls_alc_flow->tsi == tsi) {
			matching_atsc3_sls_alc_flow = to_check_atsc3_sls_alc_flow;
		}
	}
	if(!matching_atsc3_sls_alc_flow) {
		_ATSC3_SLS_ALC_FLOW_TRACE("atsc3_sls_alc_flow_find_entry_tsi: couldn't find flow in %p, count: %d, tsi: %d\n", atsc3_sls_alc_flow, atsc3_sls_alc_flow->count, tsi);
	}
	return matching_atsc3_sls_alc_flow;
}




void atsc3_route_object_set_alc_flow_and_tsi_toi(atsc3_route_object_t* atsc3_route_object, atsc3_sls_alc_flow_t* atsc3_sls_alc_flow, atsc3_alc_packet_t* atsc3_alc_packet) {
	atsc3_route_object->atsc3_sls_alc_flow = atsc3_sls_alc_flow;

	atsc3_route_object->tsi = atsc3_alc_packet->def_lct_hdr->tsi;
	atsc3_route_object->toi = atsc3_alc_packet->def_lct_hdr->toi;

	//defer any other attributes to the atsc3_route_object_lct_packet_received impl
}

atsc3_route_object_lct_packet_received_t* atsc3_route_object_add_or_update_lct_packet_received(atsc3_route_object_t* atsc3_route_object, atsc3_alc_packet_t* atsc3_alc_packet) {
	atsc3_route_object_lct_packet_received_t* atsc3_route_object_lct_packet_received = atsc3_route_object_find_lct_packet_received(atsc3_route_object, atsc3_alc_packet);
	if(!atsc3_route_object_lct_packet_received) {
		//atsc3_route_object_lct_packet_received = atsc3_route_object_lct_packet_received_new();

		atsc3_route_object_lct_packet_received = calloc(1, sizeof(atsc3_route_object_lct_packet_received_t));

		atsc3_route_object_lct_packet_received_set_attributes_from_alc_packet(atsc3_route_object_lct_packet_received, atsc3_alc_packet);
		printf("new atsc3_route_object_lct_packet_received: atsc3_route_object: %p, lct_packet_recv: %p, tsi: %d, toi: %d, start_offset: %d\n",atsc3_route_object, atsc3_route_object_lct_packet_received, atsc3_alc_packet->def_lct_hdr->tsi, atsc3_alc_packet->def_lct_hdr->toi, atsc3_alc_packet->start_offset);

		atsc3_route_object_add_atsc3_route_object_lct_packet_received(atsc3_route_object, atsc3_route_object_lct_packet_received);

#ifdef __ATSC3_ROUTE_OBJECT_PENDANTIC__
		_ATSC3_SLS_ALC_FLOW_DEBUG("atsc3_route_object_add_or_update_lct_packet_received: ADDED, tsi: %d, toi: %d, alc_packet->start_offset: %d, to_check->start_offset: %d",
								atsc3_route_object->tsi, atsc3_route_object->toi,
								atsc3_alc_packet->start_offset,
								atsc3_route_object_lct_packet_received->start_offset);
#endif

	} else {
		//increment our packet carousel count
		atsc3_route_object_lct_packet_received_update_carousel_count(atsc3_route_object_lct_packet_received, atsc3_alc_packet);
		_ATSC3_SLS_ALC_FLOW_DEBUG("atsc3_route_object_add_or_update_lct_packet_received: UPDATED, tsi: %d, toi: %d, alc_packet->start_offset: %d, to_check->start_offset: %d",
								atsc3_route_object->tsi, atsc3_route_object->toi,
								atsc3_alc_packet->start_offset,
								atsc3_route_object_lct_packet_received->start_offset);

	}


	atsc3_route_object_lct_packet_received_update_atsc3_route_object(atsc3_route_object, atsc3_route_object_lct_packet_received);

	return atsc3_route_object_lct_packet_received;
}

atsc3_route_object_lct_packet_received_t* atsc3_route_object_find_lct_packet_received(atsc3_route_object_t* atsc3_route_object, atsc3_alc_packet_t* atsc3_alc_packet) {
	atsc3_route_object_lct_packet_received_t* matching_atsc3_route_object_lct_packet_received = NULL;
	atsc3_route_object_lct_packet_received_t* to_check_atsc3_route_object_lct_packet_received = NULL;

	//jjustman-2020-07-28 - todo: refactor out this matching logic

	for(int i=0; i < atsc3_route_object->atsc3_route_object_lct_packet_received_v.count && !matching_atsc3_route_object_lct_packet_received; i++) {
		to_check_atsc3_route_object_lct_packet_received = atsc3_route_object->atsc3_route_object_lct_packet_received_v.data[i];

		if(atsc3_alc_packet->use_sbn_esi && to_check_atsc3_route_object_lct_packet_received->use_sbn_esi) {
			if(atsc3_alc_packet->sbn == to_check_atsc3_route_object_lct_packet_received->sbn && atsc3_alc_packet->esi == to_check_atsc3_route_object_lct_packet_received->esi) {
				matching_atsc3_route_object_lct_packet_received = to_check_atsc3_route_object_lct_packet_received;
			}
		} else if(atsc3_alc_packet->use_start_offset && to_check_atsc3_route_object_lct_packet_received->use_start_offset) {
			if(atsc3_alc_packet->start_offset == to_check_atsc3_route_object_lct_packet_received->start_offset) {

				_ATSC3_SLS_ALC_FLOW_DEBUG("atsc3_route_object_find_lct_packet_received: found matching for tsi: %d, toi: %d, alc_packet->start_offset: %d, to_check->start_offset: %d",
						atsc3_route_object->tsi, atsc3_route_object->toi,
						atsc3_alc_packet->start_offset,
						to_check_atsc3_route_object_lct_packet_received->start_offset);

				matching_atsc3_route_object_lct_packet_received = to_check_atsc3_route_object_lct_packet_received;
			}
		}
	}

	return matching_atsc3_route_object_lct_packet_received;
}


void atsc3_route_object_lct_packet_received_set_attributes_from_alc_packet(atsc3_route_object_lct_packet_received_t* atsc3_route_object_lct_packet_received, atsc3_alc_packet_t* atsc3_alc_packet) {
	atsc3_route_object_lct_packet_received->first_received_timestamp = gtl();
	atsc3_route_object_lct_packet_received->most_recent_received_timestamp = atsc3_route_object_lct_packet_received->first_received_timestamp;
	atsc3_route_object_lct_packet_received->codepoint = atsc3_alc_packet->def_lct_hdr->codepoint;

	//jjustman-2020-07-28 - todo - fixme
	atsc3_route_object_lct_packet_received->fec_encoding_id = atsc3_alc_packet->def_lct_hdr->codepoint;

	atsc3_route_object_lct_packet_received->use_sbn_esi = atsc3_alc_packet->use_sbn_esi;
	atsc3_route_object_lct_packet_received->sbn = atsc3_alc_packet->sbn;
	atsc3_route_object_lct_packet_received->esi = atsc3_alc_packet->esi;

	atsc3_route_object_lct_packet_received->use_start_offset = atsc3_alc_packet->use_start_offset;
	atsc3_route_object_lct_packet_received->start_offset = atsc3_alc_packet->start_offset;

	atsc3_route_object_lct_packet_received->close_object_flag = atsc3_alc_packet->close_object_flag;
	atsc3_route_object_lct_packet_received->close_session_flag = atsc3_alc_packet->close_session_flag;

	atsc3_route_object_lct_packet_received->ext_route_presentation_ntp_timestamp_set = atsc3_alc_packet->ext_route_presentation_ntp_timestamp_set;
	atsc3_route_object_lct_packet_received->ext_route_presentation_ntp_timestamp = atsc3_alc_packet->ext_route_presentation_ntp_timestamp;

	atsc3_route_object_lct_packet_received->packet_len = atsc3_alc_packet->alc_len;
	atsc3_route_object_lct_packet_received->object_len = atsc3_alc_packet->transfer_len;

#ifdef __ATSC3_ROUTE_OBJECT_PENDANTIC__

	_ATSC3_SLS_ALC_FLOW_DEBUG("atsc3_route_object_lct_packet_received_set_attributes_from_alc_packet: atsc3_alc_packet: tsi: %d, toi: %d, use_sbn_esi: %d, use_start_offset: %d, atsc3_route_object_lct_packet_received: use_sbn_esi: %d, use_start_offset: %d",
			atsc3_alc_packet->def_lct_hdr->tsi, atsc3_alc_packet->def_lct_hdr->toi,
			atsc3_alc_packet->use_sbn_esi, atsc3_alc_packet->use_start_offset,
			atsc3_route_object_lct_packet_received->use_sbn_esi, atsc3_route_object_lct_packet_received->use_start_offset);

#endif

}

void atsc3_route_object_lct_packet_received_update_carousel_count(atsc3_route_object_lct_packet_received_t* atsc3_route_object_lct_packet_received, atsc3_alc_packet_t* atsc3_alc_packet) {
	atsc3_route_object_lct_packet_received->carousel_count++;
	atsc3_route_object_lct_packet_received->most_recent_received_timestamp = gtl();

}

void atsc3_route_object_lct_packet_received_update_atsc3_route_object(atsc3_route_object_t* atsc3_route_object, atsc3_route_object_lct_packet_received_t* atsc3_route_object_lct_packet_received) {
	if(!atsc3_route_object->object_length && atsc3_route_object_lct_packet_received->object_len) {
		atsc3_route_object->object_length = atsc3_route_object_lct_packet_received->object_len;
	}
	atsc3_route_object->most_recent_atsc3_route_object_lct_packet_received = atsc3_route_object_lct_packet_received;
}


// jjustman-2020-07-14: removed - use atsc3_route_s_tsid_RS_LS_SrcFlow_ContentInfo_MediaInfo_clone intead

//void atsc3_sls_alc_flow_set_rep_id_if_null(atsc3_sls_alc_flow_t* atsc3_sls_alc_flow, char* rep_id) {
//	if(atsc3_sls_alc_flow && atsc3_sls_alc_flow->rep_id == NULL && rep_id != NULL) {
//		atsc3_sls_alc_flow->rep_id = strndup(rep_id, strlen(rep_id));
//	}
//}
//
//void atsc3_sls_alc_flow_set_lang_if_null(atsc3_sls_alc_flow_t* atsc3_sls_alc_flow, char* lang) {
//	if(atsc3_sls_alc_flow && atsc3_sls_alc_flow->lang == NULL && lang != NULL) {
//		atsc3_sls_alc_flow->lang = strndup(lang, strlen(lang));
//	}
//}


//
//atsc3_sls_alc_flow_t* atsc3_sls_alc_flow_find_entry_tsi_toi_nrt(atsc3_sls_alc_flow_v* atsc3_sls_alc_flow, uint32_t tsi, uint32_t toi) {
//	atsc3_sls_alc_flow_t* matching_atsc3_sls_alc_flow_nrt = NULL;
//	atsc3_sls_alc_flow_t* to_check_atsc3_sls_alc_flow_nrt = NULL;
//
//	for(int i=0; i < atsc3_sls_alc_flow->count && !matching_atsc3_sls_alc_flow_nrt; i++) {
//		to_check_atsc3_sls_alc_flow_nrt = atsc3_sls_alc_flow->data[i];
//		if(to_check_atsc3_sls_alc_flow_nrt->tsi == tsi && to_check_atsc3_sls_alc_flow_nrt->toi == toi) {
//			matching_atsc3_sls_alc_flow_nrt = to_check_atsc3_sls_alc_flow_nrt;
//		}
//	}
//
//	return matching_atsc3_sls_alc_flow_nrt;
//}
//
//void atsc3_sls_alc_flow_nrt_set_fdt_file_content_type_if_null(atsc3_sls_alc_flow_t* matching_atsc3_sls_alc_flow_nrt, char* fdt_file_content_type) {
//	if(matching_atsc3_sls_alc_flow_nrt && matching_atsc3_sls_alc_flow_nrt->fdt_file_content_type == NULL && fdt_file_content_type != NULL) {
//		matching_atsc3_sls_alc_flow_nrt->fdt_file_content_type = strndup(fdt_file_content_type, strlen(fdt_file_content_type));
//	}
//}


uint32_t atsc3_sls_alc_flow_get_first_tsi(atsc3_sls_alc_flow_v* atsc3_sls_alc_flow) {
	uint32_t matching_tsi = 0;
	if(atsc3_sls_alc_flow->count && atsc3_sls_alc_flow->data[0]) {
		matching_tsi = atsc3_sls_alc_flow->data[0]->tsi;
	}

	return matching_tsi;
}


uint32_t atsc3_sls_alc_flow_get_last_closed_toi(atsc3_sls_alc_flow_v* atsc3_sls_alc_flow) {
	uint32_t matching_last_closed_toi = 0;
	if(atsc3_sls_alc_flow->count && atsc3_sls_alc_flow->data[0]) {
		matching_last_closed_toi = atsc3_sls_alc_flow->data[0]->last_closed_toi;
	}

	return matching_last_closed_toi;
}



uint32_t atsc3_sls_alc_flow_get_first_toi_init(atsc3_sls_alc_flow_v* atsc3_sls_alc_flow) {
	uint32_t matching_toi_init = 0;
	if(atsc3_sls_alc_flow->count && atsc3_sls_alc_flow->data[0]) {
		matching_toi_init = atsc3_sls_alc_flow->data[0]->toi_init;
	}

	return matching_toi_init;
}
