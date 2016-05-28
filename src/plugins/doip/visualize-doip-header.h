/**
* Copyright 2017 The Open Source Research Group,
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

#ifndef __VISUALIZE_DOIP_HEADER_H
#define __VISUALIZE_DOIP_HEADER_H

#include <epan/proto.h>

#include "doip-header.h"

void
register_proto_doip_header(gint proto_doip);

void
visualize_doip_header(doip_header *, proto_tree *, gint proto_doip);

#endif /* __VISUALIZE_DOIP_HEADER_H */

