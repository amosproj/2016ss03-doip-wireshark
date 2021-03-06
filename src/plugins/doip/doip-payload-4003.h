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

#ifndef __DOIP_PAYLOAD_4003_H
#define __DOIP_PAYLOAD_4003_H

#include "doip-header.h"

void
register_proto_doip_payload_4003(gint);

void
dissect_payload_4003(doip_header *, proto_item *, packet_info *pinfo);

#endif /* __DOIP_PAYLOAD_4003_H */


