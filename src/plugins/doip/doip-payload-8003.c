/**
* Copyright 2016 The Open Source Research Group,
*                University of Erlangen-Nürnberg
*
* Licensed under the GNU AFFERO GENERAL PUBLIC LICENSE, Version 3.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     https://www.gnu.org/licenses/gpl.html
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "config.h"
#include <epan/proto.h>

#include "doip-header.h"
#include "doip-helper.h"
#include "doip-payload-8003.h"

/* Source address */
static gint hf_sa = -1; 

/* Target address */
static gint hf_ta = -1; 

/* NACK code */
static gint hf_nack = -1;

/* Previous diagnostic message data */
static gint hf_pdmd = -1;

static gint ett_diag_msg_pos_resp = -1;

/** Possible values for negative acknoledges
 * Values taken and description from ISO 13400-2:2012(E)
 * table 31.
*/
static const range_string nack_codes[] = 
{
    {0x00, 0x01, "Reserved by this part of ISO 13400"},
    {0x02, 0x02, "Invalid source address"},
    {0x02, 0x02, "Invalid source address"},
    {0x03, 0x03, "Unknown target address"},
    {0x04, 0x04, "Diagnostic message too large"},
    {0x05, 0x05, "Out of memory"},
    {0x06, 0x06, "Target unreachable"},
    {0x07, 0x07, "Unknown network"},
    {0x08, 0x08, "Transport protocol error"},
    {0x09, 0xFF, "Reserved by this part of ISO 13400"},
    {0x00, 0x00, NULL}
};

/** Values are defined in ISO 13400-2:2012(E)
* on table 39
*/
static const range_string address_values[] = {
    { 0x0000, 0x0000, "ISO/SAE reserved" },
    { 0x0001, 0x0DFF, "Vehicle manufacturer specific" },
    { 0x0E00, 0x0FFF, "Reserved for addresses of external test equipment" },
    { 0x0E00, 0x0E7F, "External legislated diagnostics test equipment (e.g. for emissions test scan-tool use)" },
    { 0x0E80, 0x0EFF, "External vehicle-manufacturer-/aftermarket-enhanced diagnostics test equipment" },
    { 0x0F00, 0x0F7F, "Internal data collection/on-board diagnostic equipment (for vehicle-manufacturer use only)" },
    { 0x0F80, 0x0FFF, "External prolonged data collection equipment (vehicle data recorders and loggers, e.g. used by insurance companies or to collect vehicle fleet data)" },
    { 0x1000, 0x7FFF, "Vehicle manufacturer specific" },
    { 0x8000, 0xCFFF, "ISO/SAE reserved" },
    { 0xD000, 0xDFFF, "Reserved for SAE Truck & Bus Control and Communication Committee" },
    { 0xE000, 0xE3FF, "ISO/SAE-reserved functional group addresses" },
    { 0xE000, 0xE000, "ISO 27145 WWH-OBD functional group address" },
    { 0xE001, 0xE3FF, "ISO/SAE reserved" },
    { 0xE400, 0xEFFF, "Vehicle-manufacturer-defined functional group logical addresses" },
    { 0xF000, 0xFFFF, "ISO/SAE reserved" },
    { 0x0000, 0x0000, NULL }
};

static void
fill_tree(doip_header *, proto_tree *tree, tvbuff_t *tvb);

static const gchar *description = "Diagnostic message negative acknowledge";
static const gchar *description_format = "Diagnostic message negative acknowledge [Source addr: %#x, Dest addr: %#x, Nack: %#x]";

/* values which will be displayed for payload type 8003 in proto_tree */
void
register_proto_doip_payload_8003(gint proto_doip)
{
    static hf_register_info hf[] =
    {
        /* prepare info for the header field
         * based on ISO 13400-2:2012(E) table 30
        */
        {
            &hf_sa,
            {
                "Source address",
                "doip.sa",
                FT_UINT16,
                BASE_HEX | BASE_RANGE_STRING,
                RVALS(address_values),
                0x0,
                "Contains the logical address of the (intended) \
                receiver of the previous diagnostic message (e.g. \
                a specific ECU on the vehicle’s networks).",
                HFILL
            }
        },
        {
            &hf_ta,
            {
                "Target address",
                "doip.ta",
                FT_UINT16,
                BASE_HEX | BASE_RANGE_STRING,
                RVALS(address_values),
                0x0,
                "Contains the logical address of the sender of the \
                previous diagnostic message (i.e. the external \
                test equipment address).",
                HFILL
            }
        },
        {
            &hf_nack,
            {
                "NACK code",
                "doip.nack",
                FT_UINT8,  
                BASE_HEX | BASE_RANGE_STRING,
                RVALS(nack_codes),
                0x0,
                "Contains the diagnostic message negative acknowledge code.",
                HFILL
            }
        },
        {
            &hf_pdmd,
            {
                "Previous diagnostic message data",
                "doip.pdmd",
                FT_BYTES,  
                BASE_NONE,
                NULL,
                0x0,
                "Currently acknowledged diagnostic message",
                HFILL
            }
        }
    };


    static gint *ett[] = 
    {
        &ett_diag_msg_pos_resp 
    };

    /* one-time registration after Wireshark is started */
    proto_register_field_array(proto_doip, hf, array_length(hf));  
    proto_register_subtree_array(ett, array_length(ett));
}

/* After a doip row is selected in Wireshark */
void
dissect_payload_8003(doip_header *header, proto_item *pitem, packet_info *pinfo)       
{
    tvbuff_t *tvb;
    proto_tree *doip_tree;

    guint16 src_addr;
    guint16 dest_addr;
    guint8 nack_code;

    /* set info column to description */

    if(get_guint16_from_message(header, &src_addr, 0)
        && get_guint16_from_message(header, &dest_addr, 2)
        && get_guint8_from_message(header, &nack_code, 4))
    {
        col_add_fstr(pinfo->cinfo, COL_INFO, description_format, src_addr, dest_addr, nack_code);
    }
    else
    {
        col_set_str(pinfo->cinfo, COL_INFO, description);
    }





    tvb = retrieve_tvbuff(header);
    /* attach a new tree to proto_item pitem */
    doip_tree = proto_item_add_subtree(pitem, ett_diag_msg_pos_resp);

    /* check for a valid tvbuff_t */
    if(doip_tree && tvb)
    {
        fill_tree(header, doip_tree, tvb);
    }
}

static void
fill_tree(doip_header *header, proto_tree *tree, tvbuff_t *tvb)
{
    /* Values taken from ISO 13400-2:2012(E) table 30
    *
    * Constants starting with prefix "REL_" indicate a relative
    * offset to a doip-messages payload.
    * In order to get the absolute offset starting from the very
    * first doip-header byte we have to calculate the
    * absolute position
    */
    const gint REL_SRC_ADDR_POS = 0;
    const gint SRC_ADDR_LEN = 2;

    const gint REL_TARGET_ADDR_POS = 2;
    const gint TARGET_ADDR_LEN = 2;

    const gint REL_NACK_CODE_POS = 4;
    const gint NACK_CODE_LEN = 1;

    const gint REL_PREV_DIAG_MSG_POS = 5;

    gint payload_len;
    gboolean has_prev_diag_msg;
    gint prev_diag_msg_len;

    payload_len = header->payload.length;
    has_prev_diag_msg = payload_len > REL_PREV_DIAG_MSG_POS;

    insert_item_to_tree(tree, hf_sa, tvb, REL_SRC_ADDR_POS, SRC_ADDR_LEN, ENC_BIG_ENDIAN);
    insert_item_to_tree(tree, hf_ta, tvb, REL_TARGET_ADDR_POS, TARGET_ADDR_LEN, ENC_BIG_ENDIAN);
    insert_item_to_tree(tree, hf_nack, tvb, REL_NACK_CODE_POS, NACK_CODE_LEN, ENC_BIG_ENDIAN);
    /* only insert this item if needed, since it is optional */
    if(has_prev_diag_msg)
    {
        prev_diag_msg_len = payload_len - REL_PREV_DIAG_MSG_POS;
        insert_item_to_tree(tree, hf_pdmd, tvb, REL_PREV_DIAG_MSG_POS, prev_diag_msg_len, ENC_NA);
    }
}

