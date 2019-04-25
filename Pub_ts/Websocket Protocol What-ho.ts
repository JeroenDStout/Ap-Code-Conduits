/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

    // ------ Client/Server connexion

import { Protocol } from './Websocket Protocol Shared'

export class ClientProperties {
    Client_Name: string;
    Client_Version: string;
}

export class ServerProperties {
    Server_Name: string;
    Server_Version: string;
    Server_Protocol: string;
}

export function create_what_ho_message(cp: ClientProperties): Uint8Array {
    let enc = new TextEncoder();
    let str = Protocol.What_Ho_Message;
    
    let json:any = {};
    json[Protocol.JSON_What_Ho_Name]             = cp.Client_Name;
    json[Protocol.JSON_What_Ho_Version]          = cp.Client_Version;
    json[Protocol.JSON_What_Ho_Protocol_Version] = Protocol.Version;
    
    return enc.encode(str + JSON.stringify(json));
}

export function parse_what_ho_response(_value: Uint8Array): ServerProperties {
    let decoder = new TextDecoder('utf-8');
    let value = decoder.decode(_value);

    if (value.length < Protocol.What_Ho_Response.length ||
        value.substr(0, Protocol.What_Ho_Response.length) != Protocol.What_Ho_Response)
    {
        throw "Invalid data in welcome mesage response";
    }

    let prop = new ServerProperties();
    let json = JSON.parse(value.substr(17));

    prop.Server_Name     = json[Protocol.JSON_What_Ho_Name];
    prop.Server_Version  = json[Protocol.JSON_What_Ho_Version];
    prop.Server_Protocol = json[Protocol.JSON_What_Ho_Protocol_Version];

    return prop;
}