/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

class MessageStateFlags {
    static Has_String           = 0x01;
    static Has_Segments         = 0x02;
    static Is_Response          = 0x04;
    
        // if response
    static Has_Succeeded        = 0x08;
    
        // if response succeeded
    static Confirm_Open_Conduit = 0x10;

        // if response failed
    static Connexion_Failure    = 0x10;

        // if not response
    static Requires_Response    = 0x08
    static Ping_Conduit         = 0x10;
}

export class Message {
    Recipient_ID = 0x0;
    Reply_To_Me_ID = 0x0;
    Flags = 0x0;

    String = "";
    Segments = new Map<number, Uint8Array>();

    set_is_response(b:boolean):void {
        if (b) { this.Flags |= MessageStateFlags.Is_Response; }
        else { this.Flags &= ~MessageStateFlags.Is_Response; }
    }
    set_has_succeeded(b:boolean):void {
        if (b) { this.Flags |= MessageStateFlags.Has_Succeeded; }
        else { this.Flags &= ~MessageStateFlags.Has_Succeeded; }
    }
    set_requires_repsonse(b:boolean):void {
        if (b) { this.Flags |= MessageStateFlags.Requires_Response; }
        else { this.Flags &= ~MessageStateFlags.Requires_Response; }
    }
    set_connexion_error(b:boolean):void {
        if (b) { this.Flags |= MessageStateFlags.Connexion_Failure; }
        else { this.Flags &= ~MessageStateFlags.Connexion_Failure; }
    }

    get_is_response(): boolean             { return 0 != (this.Flags & MessageStateFlags.Is_Response); }
    get_has_succeeded(): boolean           { return 0 != (this.Flags & MessageStateFlags.Has_Succeeded); }
    get_requires_response(): boolean       { return 0 != (this.Flags & MessageStateFlags.Requires_Response); }
    get_confirm_open_conduit(): boolean    { return 0 != (this.Flags & MessageStateFlags.Confirm_Open_Conduit); }
    get_has_connecion_failure(): boolean   { return 0 != (this.Flags & MessageStateFlags.Connexion_Failure); }

    set_segment_from_json(index: number, json: any) {
        this.Segments.set(index, (new TextEncoder()).encode(JSON.stringify(json)));
    }
    get_segment_as_json(index: number): any {
        let segment = this.Segments.get(index);
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

function to_uint16(input: number, out: Uint8Array, index: number): void {
    out[index + 0] = input & (255);
    input >>= 8
    out[index + 1] = input & (255);
}

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

function from_uint16(input: Uint8Array, index: number): number {
    let out = input[index+0];
    out += input[index+1] << 8;
    return out;
}

function from_uint8(input: Uint8Array, index: number): number {
    let out = input[index+0];
    return out;
}

export function try_parse_message(msg: Uint8Array): Message {
    let ret = new Message();
    let decoder = new TextDecoder();

            // Fixed header (0) (9 bytes)
    
    ret.Recipient_ID            = from_uint32(msg,  0);         // H+0 Recipient ID (4)
    ret.Reply_To_Me_ID          = from_uint32(msg,  4);         // H+4 Reply-To-Me ID (4)
    ret.Flags                   = from_uint8(msg,  8);          // H+8 Flags (1)

    let read_point = 9;
    
            // Optional if has string (H+9) (2+x)

    if (0 != (ret.Flags & MessageStateFlags.Has_String)) {
        let str_length          = from_uint16(msg, read_point); // St+0 String length (2)
        read_point += 2;
        
                                                                // St+2 String (x)
        ret.String = decoder.decode(msg.slice(read_point, read_point + str_length));
        read_point += str_length;
    }
    else {
        ret.String = "";
    }

            // Optional if has segments (H+9 or St+2+x) (5*n+Σx_n)
            
    if (0 != (ret.Flags & MessageStateFlags.Has_Segments)) {
        let seg_count          = from_uint8(msg, read_point);   // Sg+0 Segment count (1)
        read_point += 1;

        for (let index = 0; index < seg_count; index++) {
            let seg_index      = from_uint8(msg, read_point);   // Sg+(5*n+Σx_n)+0 Index (1)
            read_point += 1;
            
            let seg_length     = from_uint32(msg, read_point);  // Sg+(5*n+Σx_n)+1 Length (4)
            read_point += 4;

                                                                // Sg+(5*n+Σx_n)+5 Data (x_n)
            ret.Segments.set(seg_index, msg.slice(read_point, read_point + seg_length) );
            read_point += seg_length;
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

    let encoder = new TextEncoder();

    let req_size: number;
    let _msg_string: Uint8Array|undefined = undefined;
    
    if (msg.String.length > 0) {
        _msg_string = encoder.encode(msg.String);
    }

            // Fixed header, 9 bytes (0) (9 bytes)
        
    req_size = 9;
        
            // Optional if has message (H+9) (2+x)
            
    if (msg.String.length > 0) {
        let msg_string = _msg_string as Uint8Array;

        req_size += 2 + msg_string.byteLength;
        msg.Flags |= MessageStateFlags.Has_String;
    }
    
            // Optional if has segments (H+9 or St+2+x) (5*n+Σx_n)
            
    if (msg.Segments.size > 0) {
        msg.Segments.forEach((value, index) => {
            req_size += 5 + value.byteLength;
        });
        msg.Flags |= MessageStateFlags.Has_Segments;
    }

        // Create response
    let out_data = new Uint8Array(req_size);

            // Fixed header (0) (9 bytes)
            
    to_uint32(msg.Recipient_ID,    out_data, 0);          // H+0 Recipient ID (4)
    to_uint32(msg.Reply_To_Me_ID,  out_data, 4);          // H+4 Reply-To-Me ID (4)
    to_uint8(msg.Flags,            out_data, 8);          // H+8 Flags (1)

    let write_point = 9;
            
            // Optional if has string (H+9) (2+x)
            
    if (msg.String.length > 0) {
        let msg_string = _msg_string as Uint8Array;

        to_uint16(msg_string.byteLength,  out_data, write_point);   // St+0 String length (2)
        write_point += 2;
        
        for (let read = 0; read < msg_string.byteLength; read++) {  // St+2 String (x)
            out_data[write_point] = msg_string[read];
            write_point += 1;
        }
        write_point += msg_string.byteLength;
    }
            // Optional if has segments (H+9 or St+2+x) (5*n+Σx_n)

    if (msg.Segments.size > 0) {
        to_uint8(msg.Segments.size, out_data, write_point);       // Sg+0 Segment count (1)
        write_point += 1;

        msg.Segments.forEach((value, index) => {
            to_uint8(index, out_data, write_point);               // Sg+(5*n+Σx_n)+0 Index (1)
            write_point += 1;

            to_uint32(value.byteLength, out_data, write_point);   // Sg+(5*n+Σx_n)+1 Size (4)
            write_point += 1;
        
            for (let read = 0; read < value.byteLength; read++) { // Sg+(5*n+Σx_n)+5 Data (x_n)
                out_data[write_point] = value[read];
                write_point += 1;
            }
        });
    }
    
    return out_data;
}