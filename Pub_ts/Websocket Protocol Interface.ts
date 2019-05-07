/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

export interface IMessage {
    String: string|undefined;
    Segments: Map<string, Uint8Array>;

    set_is_response(b:boolean):void;
    set_is_OK(b:boolean):void;
    set_conform_open_conduit(b:boolean):void;
    set_accepts_response(b:boolean):void;
    set_connexion_failure(b:boolean):void;
    set_no_response_expected(b:boolean):void;
    set_timed_out(b:boolean):void;
    set_receiver_will_not_handle(b:boolean):void;

    get_is_response(): boolean;
    get_is_OK(): boolean;
    get_confirm_open_conduit(): boolean;
    get_accepts_response(): boolean;
    get_connexion_failure(): boolean;
    get_no_response_expected(): boolean;
    get_timed_out(): boolean;
    get_receiver_will_not_handle(): boolean;

    set_segment_from_json(name: string, json: any): void;
    get_segment_as_json(name: string): any;
}