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
#include "doip-payload-4002.h"

/* Node type */
static gint hf_nd = -1;

/* Max. concurrent TCP_DATA sockets*/
static gint hf_mcts = -1;

/* Currently open TCP_DATA sockets*/
static gint hf_ncts = -1;

/* Max. data size*/
static gint hf_mds = -1;

static gint ett_doip_status_response = -1;

static void
fill_tree(proto_tree *tree, tvbuff_t *tvb, guint32 payloadLength);

static const gchar *description = "DoIP status response";
static const gchar *description_format = "DoIP status response [Node type: %#x, open TCP sockets: %#x, max. data size: %#x]";

/** Values are defined in ISO 13400-2:2012(E)
* on table 37
*/
static const range_string node_types[] = {
    { 0x00, 0x00, "DoIP gateway" },
    { 0x01, 0x01, "DoIP node" },
    { 0x02, 0xFF, "reserved by this part of ISO 13400" },
    { 0x00, 0x00, NULL }
};

/* values which will be displayed for payload type 4002 in proto_tree */
void
register_proto_doip_payload_4002(gint proto_doip)
{
    static hf_register_info hf[] =
    {
    /*prepare info for the header field based on ISO 13400 - 2:2012(E) table 37 */
    {
        &hf_nd,
        {
        "Node type",
        "doip.nd",
        FT_UINT8,
        BASE_HEX | BASE_RANGE_STRING,
        RVALS(node_types),
        0x0,
        "Identifies whether the contaced DoIP instance is either a DoIP node or a DoIP gateway.",
        HFILL
        }
    },
    {
        &hf_mcts,
        {
        "Max. concurrent TCP_DATA sockets",
        "doip.mcts",
        FT_UINT8,
        BASE_DEC,
        NULL,
        0x0,
        "Represents the maximum number of concurrent TCP_DATA sockets allowed with this DoIP entity, excluding the reserve socket required for socket handling.",
        HFILL
        }
    },
    {
        &hf_ncts,
        {
        "Currently open TCP_DATA sockets",
        "doip.ncts",
        FT_UINT8,
        BASE_DEC,
        NULL,
        0x0,
        "Number of currently established sockets.",
        HFILL
        }
    },
    {
        &hf_mds,
        {
            "Max. data size",
            "doip.mds",
            FT_UINT32,
            BASE_DEC,
        NULL,
        0x0,
        "Maximum size of one logical request that this DoIP entity can process.",
        HFILL
        }
    }
    };

    static gint *ett[] =
    {
        &ett_doip_status_response,
    };
        
        /* one-time registration after Wireshark is started */
    proto_register_field_array(proto_doip, hf, array_length(hf));
    proto_register_subtree_array(ett, array_length(ett));
}

/* After a doip row is selected in Wireshark */
void
dissect_payload_4002(doip_header *header, proto_item *pitem, packet_info *pinfo)
{
    tvbuff_t *tvb;
    proto_tree *doip_tree;
    guint32 payloadLength;

    guint8 node_type;
    guint8 open_tcp_socks;
    guint32 max_data_size;


    /* get the length of the payload */
    payloadLength = header->payload.length;

    tvb = retrieve_tvbuff(header);
    /* attach a new tree to proto_item pitem */
    doip_tree = proto_item_add_subtree(pitem, ett_doip_status_response);

    /* set info column to description */
//static const gchar *description_format = "DoIP status response [Node type: %#x, open TCP sockets: %#x, max. data size: %#x";

    if(get_guint8_from_message(header, &node_type, 0)
        && get_guint8_from_message(header, &open_tcp_socks, 2)
        && get_guint32_from_message(header, &max_data_size, 3))
    {
        col_add_fstr(pinfo->cinfo, COL_INFO, description_format, node_type, open_tcp_socks, max_data_size);
    }
    else
    {
        col_set_str(pinfo->cinfo, COL_INFO, description);
    }

    /* check for a valid tvbuff_t */
    if (doip_tree && tvb)
    {
        fill_tree(doip_tree, tvb, payloadLength);
    }
}

static void
fill_tree(proto_tree *tree, tvbuff_t *tvb, guint32 payloadLength)
{
    /* Values taken from ISO 13400-2:2012(E) table 37
    *
    * Constants starting with prefix "REL_" indicate a relative
    * offset to a doip-messages payload.
    * In order to get the absolute offset starting from the very
    * first doip-header byte we have to calculate the
    * absolute position
    */
    const gint REL_ND_POS = 0;
    const gint ND_LEN = 1;

    const gint REL_MCTS_POS = 1;
    const gint MCTS_LEN = 1;

    const gint REL_NCTS_POS = 2;
    const gint NCTS_LEN = 1;

    const gint REL_MDS_POS = 3;
    const gint MDS_LEN = 4;

    gboolean mds_is_present = ((gint) payloadLength) >= (REL_MDS_POS + MDS_LEN);

    insert_item_to_tree(tree, hf_nd, tvb, REL_ND_POS, ND_LEN, ENC_BIG_ENDIAN);
    insert_item_to_tree(tree, hf_mcts, tvb, REL_MCTS_POS, MCTS_LEN, ENC_BIG_ENDIAN);
    insert_item_to_tree(tree, hf_ncts, tvb, REL_NCTS_POS, NCTS_LEN, ENC_BIG_ENDIAN);
    /* only insert this item if needed, since it is optional */
    if (mds_is_present)
    {
        insert_item_to_tree(tree, hf_mds, tvb, REL_MDS_POS, MDS_LEN, ENC_BIG_ENDIAN);
    }
}




