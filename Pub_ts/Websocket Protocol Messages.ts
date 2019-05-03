/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

class MessageStateFlags {
    static Has_String               = 0x01;
    static Has_Segments             = 0x02;
    static Is_Response              = 0x04;

        // if response...
    static Is_OK                    = 0x08;

        // if response is OK...
    static Confirm_Opened_Conduit   = 0x10;

        // if response not OK...
    static Connexion_Failure        = 0x10;
    static No_response_Expected     = 0x20;
    static Timed_Out                = 0x40;
    static Receiver_Will_Not_Handle = 0x80;

        // if not response...
    static Accepts_Response         = 0x08;
    static Ping_Conduit             = 0x10;
    static Close_Conduit            = 0x20;
}

export class Message {
    Recipient_ID = 0x0;
    Reply_To_Me_ID = 0x0;
    Opened_Conduit_ID = 0x0;
    Flags = 0x0;

    String: string|undefined = undefined;
    Segments = new Map<string, Uint8Array>();

    set_is_response(b:boolean):void {
        if (b) { this.Flags |= MessageStateFlags.Is_Response; }
        else { this.Flags &= ~MessageStateFlags.Is_Response; }
    }
    set_is_OK(b:boolean):void {
        if (b) { this.Flags |= MessageStateFlags.Is_OK; }
        else { this.Flags &= ~MessageStateFlags.Is_OK; }
    }
    set_conform_open_conduit(b:boolean):void {
        if (b) { this.Flags |= MessageStateFlags.Confirm_Opened_Conduit; }
        else { this.Flags &= ~MessageStateFlags.Confirm_Opened_Conduit; }
    }
    set_accepts_response(b:boolean):void {
        if (b) { this.Flags |= MessageStateFlags.Accepts_Response; }
        else { this.Flags &= ~MessageStateFlags.Accepts_Response; }
    }
    set_connexion_failure(b:boolean):void {
        if (b) { this.Flags |= MessageStateFlags.Connexion_Failure; }
        else { this.Flags &= ~MessageStateFlags.Connexion_Failure; }
    }
    set_no_response_expected(b:boolean):void {
        if (b) { this.Flags |= MessageStateFlags.No_response_Expected; }
        else { this.Flags &= ~MessageStateFlags.No_response_Expected; }
    }
    set_timed_out(b:boolean):void {
        if (b) { this.Flags |= MessageStateFlags.Timed_Out; }
        else { this.Flags &= ~MessageStateFlags.Timed_Out; }
    }
    set_receiver_will_not_handle(b:boolean):void {
        if (b) { this.Flags |= MessageStateFlags.Receiver_Will_Not_Handle; }
        else { this.Flags &= ~MessageStateFlags.Receiver_Will_Not_Handle; }
    }

    get_is_response(): boolean                { return 0 != (this.Flags & MessageStateFlags.Is_Response); }
    get_is_OK(): boolean                      { return 0 != (this.Flags & MessageStateFlags.Is_OK); }
    get_confirm_open_conduit(): boolean       { return 0 != (this.Flags & MessageStateFlags.Confirm_Opened_Conduit); }
    get_accepts_response(): boolean           { return 0 != (this.Flags & MessageStateFlags.Accepts_Response); }
    get_connexion_failure(): boolean          { return 0 != (this.Flags & MessageStateFlags.Connexion_Failure); }
    get_no_response_expected(): boolean       { return 0 != (this.Flags & MessageStateFlags.No_response_Expected); }
    get_timed_out(): boolean                  { return 0 != (this.Flags & MessageStateFlags.Timed_Out); }
    get_receiver_will_not_handle(): boolean   { return 0 != (this.Flags & MessageStateFlags.Receiver_Will_Not_Handle); }

    set_segment_from_json(name: string, json: any) {
        this.Segments.set(name, (new TextEncoder()).encode(JSON.stringify(json)));
    }
    get_segment_as_json(name: string): any {
        let segment = this.Segments.get(name);
        return JSON.parse((new TextDecoder()).decode(segment));
    }
}

function to_uint32(input: number, out: Uint8Array, index: number): void {
    out[index+0] = input & (255);
    input >>= 8
    out[index+1] = input & (255);
    input >>= 8
    out[index+2] = input & (255);
    input >>= 8
    out[index+3] = input & (255);
}

//function to_uint16(input: number, out: Uint8Array, index: number): void {
//    out[index + 0] = input & (255);
//    input >>= 8
//    out[index + 1] = input & (255);
//}

function to_uint8(input: number, out: Uint8Array, index: number): void {
    out[index + 0] = input & (255);
}

function from_uint32(input: Uint8Array, index: number): number {
    let out = input[index+0];
    out += input[index+1] << 8;
    out += input[index+2] << 16;
    out += input[index+3] << 24;
    return out;
}

//function from_uint16(input: Uint8Array, index: number): number {
//    let out = input[index+0];
//    out += input[index+1] << 8;
//    return out;
//}

function from_uint8(input: Uint8Array, index: number): number {
    let out = input[index+0];
    return out;
}

export function try_parse_message(msg: Uint8Array): Message {
    let ret = new Message();
    let decoder = new TextDecoder();
    
    let idx = 0;

    ret.Flags                   = from_uint8(msg, idx);         // Flags (1)
    idx += 1;

    ret.Recipient_ID            = from_uint32(msg, idx);        // Recipient (4)
    idx += 4;

    if (ret.get_accepts_response()) {
        ret.Reply_To_Me_ID      = from_uint32(msg, idx);        // Response ID (4)
        idx += 4;
    }
    else {
        ret.Reply_To_Me_ID      = -1;
    }

    if (ret.get_confirm_open_conduit()) {
        ret.Opened_Conduit_ID   = from_uint32(msg, idx);        // Conduit ID (4)
        idx += 4;
    }
    else {
        ret.Opened_Conduit_ID   = -1;
    }

    if (0 != (ret.Flags & MessageStateFlags.Has_String)) {
        let itt = idx;
        while (msg[itt] != 0) {
            itt += 1;
        }
        ret.String = decoder.decode(msg.slice(idx, itt));       // String value (n, ends with \0)
        idx = itt + 1;
    }
    else {
        ret.String = undefined;
    }

    if (0 != (ret.Flags & MessageStateFlags.Has_Segments)) {
        let segment_count = from_uint8(msg, idx);               // Segment count (1)
        idx += 1;

        for (let seg = 0; seg < segment_count; seg++) {
            let itt = idx;
            while (msg[itt] != 0) {
                itt += 1;
            }
            let name = decoder.decode(msg.slice(idx, itt));     // Segment name (n, ends with \0)
            idx = itt + 1;

            let seg_length  = from_uint32(msg, idx);            // Segment length (4)
            idx += 4;
                                                                // Segment data (n)
            ret.Segments.set(name, msg.slice(idx, idx + seg_length));
            idx += seg_length;
        }
    }
    
    return ret;
}

export function try_stringify_message(msg: Message): Uint8Array {
    if (msg.Recipient_ID > 0xFFFFFFFF) {
        throw "Recipient ID out of range";
    }
    if (msg.Reply_To_Me_ID > 0xFFFFFFFF) {
        throw "Reply-to-me ID out of range";
    }

    console.log(msg);

    let encoder = new TextEncoder();

    let req_size: number;
    let _msg_string: Uint8Array|undefined = undefined;
    let flags = msg.Flags;
    
    if (msg.String !== undefined && (msg.String as string).length > 0) {
        let fin_string = msg.String as string;
        fin_string += '\0';
        _msg_string = encoder.encode(fin_string);
    }
    
    req_size = 5;                                           // Flags + Recipient ID (5)

    if (msg.get_accepts_response()) {
        req_size += 4;                                      // Response ID (4)
    }
    if (msg.get_confirm_open_conduit()) {
        req_size += 4;                                      // Conduit ID (4)
    }

    if (msg.String !== undefined && msg.String.length > 0) {
        flags |= MessageStateFlags.Has_String;
        req_size += (_msg_string as Uint8Array).byteLength; // String value ending in \0 (n + 1)
    }

    let uint8names = new Map<string, Uint8Array>();
    if (msg.Segments.size > 0) {
        flags |= MessageStateFlags.Has_Segments;
        req_size += 1;                                      // Segment count (1)
        
        msg.Segments.forEach((value, index) => {
            let name = encoder.encode(index);
            uint8names[index] = name;
            req_size += name.byteLength + 1;                // String name ending in \0 (n + 1)
            req_size += 4;                                  // Segment size (4)
            req_size += value.byteLength;                   // Segment data (n)
        });
    }

        // Create response
    let out_data = new Uint8Array(req_size);
            
    let idx = 0;

    to_uint8(flags, out_data, idx);                         // Flags (1)
    idx += 1;

    to_uint32(msg.Recipient_ID,out_data, idx);              // Recipient ID (4)
    idx += 4;

    if (msg.get_accepts_response()) {
        to_uint32(msg.Reply_To_Me_ID, out_data, idx);       // Response ID (4)
        idx += 4;
    }
    if (msg.get_confirm_open_conduit()) {
        to_uint32(msg.Opened_Conduit_ID, out_data, idx);    // Conduit ID (4)
        idx += 4;
    }
    
    if (_msg_string !== undefined) {
        let msg_string = _msg_string as Uint8Array;         // String value ending in \0 (n + 1)
        for (let read = 0; read < msg_string.byteLength; read++) {
            out_data[idx] = msg_string[read];
            idx += 1;
        }
    }
    
    if (msg.Segments.size > 0) {
        to_uint8(msg.Segments.size, out_data, idx);         // Segment count (1)
        idx += 1;

        msg.Segments.forEach((value, index) => {
            let name = uint8names[index];

            for (let read = 0; read < name.byteLength; read++) {  // String name ending in \0 (n + 1)
                out_data[idx] = name[read];
                idx += 1;
            }
            out_data[idx] = 0;
            idx += 1;           
            
            to_uint32(value.byteLength, out_data, idx);           // Segment size (4)
            idx += 4;
        
            for (let read = 0; read < value.byteLength; read++) { // Segment data (n)
                out_data[idx] = value[read];
                idx += 1;
            }
        });
    }
    
    return out_data;
}