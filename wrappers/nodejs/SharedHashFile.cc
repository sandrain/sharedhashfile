/*
 * ============================================================================
 * Copyright (c) 2014 Hardy-Francis Enterprises Inc.
 * This file is part of SharedHashFile.
 *
 * SharedHashFile is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * SharedHashFile is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see www.gnu.org/licenses/.
 * ----------------------------------------------------------------------------
 * To use SharedHashFile in a closed-source product, commercial licenses are
 * available; email office [@] sharedhashfile [.] com for more information.
 * ============================================================================
 */

#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif
#include <node.h>
#include <node_buffer.h>

#include <SharedHashFile.hpp>

using namespace v8;
using namespace node;

// Useful links WRT creating nodejs wrappers:
// See http://luismreis.github.io/node-bindings-guide/docs/returning.html
// See https://github.com/TooTallNate/node-gyp
// See https://github.com/TooTallNate/node-gyp/wiki/%22binding.gyp%22-files-out-in-the-wild

// todo: Consider handling native javascript values as sharedhashfile values; faster? See https://www.npmjs.org/package/hashtable implementation; "values can be any javascript type, including objects".
// todo: This code probably needs to be reviewed by somebody who is experienced creating nodejs wrappers for C/C++ code!

// External string resource that glues shared memory and v8::String
class CustomExternalStringResource : public String::ExternalStringResource {
private:
    const uint16_t * shf_item_addr_;
    size_t           shf_item_addr_len_;

public:
    CustomExternalStringResource(void * shf_item_addr, uint32_t shf_item_addr_len);
    ~CustomExternalStringResource();

    void             Dispose();
    const uint16_t * data() const;
    size_t           length() const;
};

CustomExternalStringResource::CustomExternalStringResource(void * shf_item_addr, uint32_t shf_item_addr_len) {
    shf_item_addr_     = SHF_CAST(const uint16_t *, shf_item_addr);
    shf_item_addr_len_ =                            shf_item_addr_len;
}

CustomExternalStringResource::~CustomExternalStringResource() { }

void CustomExternalStringResource::Dispose() {
    /* Do nothing; no need to dispose of SharedHashFile shared memory :-) */
}

const uint16_t * CustomExternalStringResource::data() const {
    return this->shf_item_addr_;
}

size_t CustomExternalStringResource::length() const {
    return this->shf_item_addr_len_;
}

class CustomExternalAsciiStringResource : public String::ExternalAsciiStringResource {
private:
    char   * shf_item_addr_;
    size_t   shf_item_addr_len_;

public:
    CustomExternalAsciiStringResource(void * shf_item_addr, uint32_t shf_item_addr_len);
    ~CustomExternalAsciiStringResource();

    void         Dispose();
    const char * data() const;
    size_t       length() const;
};

CustomExternalAsciiStringResource::CustomExternalAsciiStringResource(void * shf_item_addr, uint32_t shf_item_addr_len) {
    shf_item_addr_     = SHF_CAST(char *, shf_item_addr);
    shf_item_addr_len_ =                  shf_item_addr_len;
}

CustomExternalAsciiStringResource::~CustomExternalAsciiStringResource() { }

void CustomExternalAsciiStringResource::Dispose() {
    /* Do nothing; no need to dispose of SharedHashFile shared memory :-) */
}

const char * CustomExternalAsciiStringResource::data() const {
    return this->shf_item_addr_;
}

size_t CustomExternalAsciiStringResource::length() const {
    return this->shf_item_addr_len_;
}

#define SHF_VALIDATE_ARG_COUNT_REQUIRED(ARGS_REQUIRED) \
    int32_t shf_args_required = ARGS_REQUIRED; \
    if (shf_args_required != args.Length()) { \
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments"))); \
        return scope.Close(Undefined()); \
    }

#define SHF_VALIDATE_ARG_IS_STRING(ARG) \
    if (!args[ARG]->IsString()) { \
        ThrowException(Exception::TypeError(String::New("Wrong argument type; must be string"))); \
        return scope.Close(Undefined()); \
    } \
    String::Utf8Value arg##ARG(args[ARG]); \
    if (0 == arg##ARG.length()) { \
        ThrowException(Exception::TypeError(String::New("Wrong argument length; must be > 0"))); \
        return scope.Close(Undefined()); \
    }

#define SHF_VALIDATE_ARG_IS_INT32(ARG) \
    if (!args[ARG]->IsNumber()) { \
        ThrowException(Exception::TypeError(String::New("Wrong argument type; must be number; int32"))); \
        return scope.Close(Undefined()); \
    } \
    /* todo: slower? uint32_t arg##ARG = args[ARG]->ToInt32()->Value(); */ \
    uint32_t arg##ARG = args[ARG]->Int32Value();

#define SHF_VALIDATE_ARG_IS_ARRAY(ARG) \
    if (!args[ARG]->IsArray()) { \
        ThrowException(Exception::TypeError(String::New("Wrong argument type; must be array"))); \
        return scope.Close(Undefined()); \
    } \
    Handle<Array> arg##ARG = Handle<Array>::Cast(args[ARG]);

#define SHF_GET_SHAREDHASHFILE_OBJ() \
    sharedHashFile * obj = ObjectWrap::Unwrap<sharedHashFile>(args.This());

#define SHF_HANDLE_SCOPE() \
    HandleScope scope;

#define SHF_DUMMY_BYTES (4096)

static char shfDummyBytes[SHF_DUMMY_BYTES];

class sharedHashFile : public node::ObjectWrap {
public:
    static void Init(v8::Handle<v8::Object> target);

private:
    sharedHashFile();
    ~sharedHashFile();

    static v8::Handle<v8::Value> New               (const v8::Arguments& args);
    static v8::Handle<v8::Value> AttachExisting    (const v8::Arguments& args);
    static v8::Handle<v8::Value> Attach            (const v8::Arguments& args);
    static v8::Handle<v8::Value> Uid               (const v8::Arguments& args);
    static v8::Handle<v8::Value> MakeHash          (const v8::Arguments& args);
    static v8::Handle<v8::Value> DelKeyVal         (const v8::Arguments& args);
    static v8::Handle<v8::Value> DelUidVal         (const v8::Arguments& args);
    static v8::Handle<v8::Value> PutKeyVal         (const v8::Arguments& args);
    static v8::Handle<v8::Value> GetKeyVal         (const v8::Arguments& args);
    static v8::Handle<v8::Value> GetUidVal         (const v8::Arguments& args);
    static v8::Handle<v8::Value> DebugGetGarbage   (const v8::Arguments& args);
    static v8::Handle<v8::Value> DebugVerbosityLess(const v8::Arguments& args);
    static v8::Handle<v8::Value> DebugVerbosityMore(const v8::Arguments& args);
    static v8::Handle<v8::Value> SetDataNeedFactor (const v8::Arguments& args);
    static v8::Handle<v8::Value> QueueNewItem      (const v8::Arguments& args);
    static v8::Handle<v8::Value> QueuePutItem      (const v8::Arguments& args);
    static v8::Handle<v8::Value> QueueNewName      (const v8::Arguments& args);
    static v8::Handle<v8::Value> QueueGetName      (const v8::Arguments& args);
    static v8::Handle<v8::Value> QueuePushHead     (const v8::Arguments& args);
    static v8::Handle<v8::Value> QueuePushHeadData (const v8::Arguments& args); /* QueuePutItem() + QueuePushHead() */
    static v8::Handle<v8::Value> QueuePushPull     (const v8::Arguments& args);
    static v8::Handle<v8::Value> QueuePullTail     (const v8::Arguments& args);
    static v8::Handle<v8::Value> QueueTakeItem     (const v8::Arguments& args);
    static v8::Handle<v8::Value> Dummy1            (const v8::Arguments& args);
    static v8::Handle<v8::Value> Dummy2            (const v8::Arguments& args);
    static v8::Handle<v8::Value> Dummy3a           (const v8::Arguments& args);
    static v8::Handle<v8::Value> Dummy3b           (const v8::Arguments& args);
    static v8::Handle<v8::Value> Dummy4            (const v8::Arguments& args);
    static v8::Handle<v8::Value> Dummy5            (const v8::Arguments& args);
    static v8::Handle<v8::Value> Dummy6            (const v8::Arguments& args);
    static v8::Handle<v8::Value> Dummy7a           (const v8::Arguments& args);
    static v8::Handle<v8::Value> Dummy7b           (const v8::Arguments& args);
    static v8::Handle<v8::Value> Dummy7c           (const v8::Arguments& args);
    static v8::Handle<v8::Value> Dummy8            (const v8::Arguments& args);
    static v8::Handle<v8::Value> Dummy9            (const v8::Arguments& args);

    SharedHashFile * shf;
};

sharedHashFile::sharedHashFile() {};
sharedHashFile::~sharedHashFile() {};

void
sharedHashFile::Init(Handle<Object> target) {
    SHF_DEBUG("%s()\n", __FUNCTION__);

    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    tpl->SetClassName(String::NewSymbol("sharedHashFile"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1); // todo: figure out what this is good for! see: http://stackoverflow.com/questions/16600735/what-is-an-internal-field-count-and-what-is-setinternalfieldcount-used-for

    // Prototype
    tpl->PrototypeTemplate()->Set(String::NewSymbol("attachExisting"    ), FunctionTemplate::New(AttachExisting    )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("attach"            ), FunctionTemplate::New(Attach            )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("uid"               ), FunctionTemplate::New(Uid               )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("makeHash"          ), FunctionTemplate::New(MakeHash          )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("delKeyVal"         ), FunctionTemplate::New(DelKeyVal         )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("delUidVal"         ), FunctionTemplate::New(DelUidVal         )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("putKeyVal"         ), FunctionTemplate::New(PutKeyVal         )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("getKeyVal"         ), FunctionTemplate::New(GetKeyVal         )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("getUidVal"         ), FunctionTemplate::New(GetUidVal         )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("debugGetGarbage"   ), FunctionTemplate::New(DebugGetGarbage   )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("debugVerbosityLess"), FunctionTemplate::New(DebugVerbosityLess)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("debugVerbosityMore"), FunctionTemplate::New(DebugVerbosityMore)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("setDataNeedFactor" ), FunctionTemplate::New(SetDataNeedFactor )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("queueNewItem"      ), FunctionTemplate::New(QueueNewItem      )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("queuePutItem"      ), FunctionTemplate::New(QueuePutItem      )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("queueNewName"      ), FunctionTemplate::New(QueueNewName      )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("queueGetName"      ), FunctionTemplate::New(QueueGetName      )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("queuePushHead"     ), FunctionTemplate::New(QueuePushHead     )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("queuePushHeadData" ), FunctionTemplate::New(QueuePushHeadData )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("queuePushPull"     ), FunctionTemplate::New(QueuePushPull     )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("queuePullTail"     ), FunctionTemplate::New(QueuePullTail     )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("queueTakeItem"     ), FunctionTemplate::New(QueueTakeItem     )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("dummy1"            ), FunctionTemplate::New(Dummy1            )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("dummy2"            ), FunctionTemplate::New(Dummy2            )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("dummy3a"           ), FunctionTemplate::New(Dummy3a           )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("dummy3b"           ), FunctionTemplate::New(Dummy3b           )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("dummy4"            ), FunctionTemplate::New(Dummy4            )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("dummy5"            ), FunctionTemplate::New(Dummy5            )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("dummy6"            ), FunctionTemplate::New(Dummy6            )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("dummy7a"           ), FunctionTemplate::New(Dummy7a           )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("dummy7b"           ), FunctionTemplate::New(Dummy7b           )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("dummy7c"           ), FunctionTemplate::New(Dummy7c           )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("dummy8"            ), FunctionTemplate::New(Dummy8            )->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("dummy9"            ), FunctionTemplate::New(Dummy9            )->GetFunction());

    Persistent<Function> constructor = Persistent<Function>::New(tpl->GetFunction());
    target->Set(String::NewSymbol("sharedHashFile"), constructor);
}

Handle<Value>
sharedHashFile::New(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(0);

    sharedHashFile * obj = new sharedHashFile();
    obj->shf = new SharedHashFile;
    obj->Wrap(args.This());

    return args.This();
}

Handle<Value>
sharedHashFile::AttachExisting(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(2);
    SHF_VALIDATE_ARG_IS_STRING(0);
    SHF_VALIDATE_ARG_IS_STRING(1);
    SHF_GET_SHAREDHASHFILE_OBJ();

    uint32_t value = obj->shf->AttachExisting(*arg0, *arg1);

    return scope.Close(Number::New(value));
}

Handle<Value>
sharedHashFile::Attach(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(2);
    SHF_VALIDATE_ARG_IS_STRING(0);
    SHF_VALIDATE_ARG_IS_STRING(1);
    SHF_GET_SHAREDHASHFILE_OBJ();

    uint32_t value = obj->shf->Attach(*arg0, *arg1);

    return scope.Close(Number::New(value));
}

Handle<Value>
sharedHashFile::Uid(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(0);

    return scope.Close(Number::New(shf_uid));
}

Handle<Value>
sharedHashFile::MakeHash(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(1);
    SHF_VALIDATE_ARG_IS_STRING(0);
    SHF_GET_SHAREDHASHFILE_OBJ();

    obj->shf->MakeHash(*arg0, arg0.length());

    return scope.Close(Undefined());
}

Handle<Value>
sharedHashFile::PutKeyVal(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(2);
    SHF_VALIDATE_ARG_IS_STRING(0);
    SHF_VALIDATE_ARG_IS_STRING(1);
    SHF_GET_SHAREDHASHFILE_OBJ();

    obj->shf->MakeHash(*arg0, arg0.length());
    uint32_t uid = obj->shf->PutKeyVal(*arg1, arg1.length());

    return scope.Close(Number::New(uid));
}

Handle<Value>
sharedHashFile::DelKeyVal(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(1);
    SHF_VALIDATE_ARG_IS_STRING(0);
    SHF_GET_SHAREDHASHFILE_OBJ();

    obj->shf->MakeHash(*arg0, arg0.length());
    uint32_t value = obj->shf->DelKeyVal();

    return scope.Close(Number::New(value));
}

Handle<Value>
sharedHashFile::DelUidVal(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(1);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_GET_SHAREDHASHFILE_OBJ();

    uint32_t uid = arg0;
    uint32_t value = obj->shf->DelUidVal(uid);

    return scope.Close(Number::New(value));
}

Handle<Value>
sharedHashFile::GetKeyVal(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(1);
    SHF_VALIDATE_ARG_IS_STRING(0);
    SHF_GET_SHAREDHASHFILE_OBJ();

    obj->shf->MakeHash(*arg0, arg0.length());
    if (0 == obj->shf->GetKeyValCopy()) { return scope.Close(Undefined()); }
    else                                { return scope.Close(String::New(shf_val, shf_val_len)); }
}

Handle<Value>
sharedHashFile::GetUidVal(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(1);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_GET_SHAREDHASHFILE_OBJ();

    int32_t uid = arg0;
    if (0 == obj->shf->GetUidValCopy(uid)) { return scope.Close(Undefined()); }
    else                                   { return scope.Close(String::New(shf_val, shf_val_len)); }
}

Handle<Value>
sharedHashFile::DebugGetGarbage(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(0);
    SHF_GET_SHAREDHASHFILE_OBJ();

    uint64_t bytes_marked_as_deleted = obj->shf->DebugGetGarbage();

    return scope.Close(Number::New(bytes_marked_as_deleted));
}

Handle<Value>
sharedHashFile::DebugVerbosityLess(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(0);
    SHF_GET_SHAREDHASHFILE_OBJ();

    obj->shf->DebugVerbosityLess();

    return scope.Close(Undefined());
}

Handle<Value>
sharedHashFile::DebugVerbosityMore(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(0);
    SHF_GET_SHAREDHASHFILE_OBJ();

    obj->shf->DebugVerbosityLess();

    return scope.Close(Undefined());
}

Handle<Value>
sharedHashFile::SetDataNeedFactor(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(1);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_GET_SHAREDHASHFILE_OBJ();

    obj->shf->SetDataNeedFactor(arg0);

    return scope.Close(Undefined());
}

Handle<Value>
sharedHashFile::QueueNewItem(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(1);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_GET_SHAREDHASHFILE_OBJ();

    int32_t data_size = arg0;
    int32_t uid = obj->shf->QueueNewItem(data_size);

    return scope.Close(Number::New(uid));
}

Handle<Value>
sharedHashFile::QueuePutItem(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(2);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_VALIDATE_ARG_IS_STRING(1);
    SHF_GET_SHAREDHASHFILE_OBJ();

    int32_t uid = arg0;
    obj->shf->QueuePutItem(uid, *arg1, arg1.length());

    return scope.Close(Undefined());
}

Handle<Value>
sharedHashFile::QueueNewName(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(1);
    SHF_VALIDATE_ARG_IS_STRING(0);
    SHF_GET_SHAREDHASHFILE_OBJ();

    int32_t uid = obj->shf->QueueNewName(*arg0, arg0.length());

    return scope.Close(Number::New(uid));
}

Handle<Value>
sharedHashFile::QueueGetName(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(1);
    SHF_VALIDATE_ARG_IS_STRING(0);
    SHF_GET_SHAREDHASHFILE_OBJ();

    int32_t uid = obj->shf->QueueGetName(*arg0, arg0.length());

    return scope.Close(Number::New(uid));
}

Handle<Value>
sharedHashFile::QueuePushHead(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(2);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_VALIDATE_ARG_IS_INT32(1);
    SHF_GET_SHAREDHASHFILE_OBJ();

    int32_t uid_head = arg0;
    int32_t uid_item = arg1;
    obj->shf->QueuePushHead(uid_head, uid_item);

    return scope.Close(Undefined());
}

Handle<Value>
sharedHashFile::QueuePushHeadData(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(3);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_VALIDATE_ARG_IS_INT32(1);
    SHF_VALIDATE_ARG_IS_STRING(2);
    SHF_GET_SHAREDHASHFILE_OBJ();

    int32_t uid_head = arg0;
    int32_t uid_item = arg1;
    obj->shf->QueuePutItem (uid_item, *arg2, arg2.length());
    obj->shf->QueuePushHead(uid_head, uid_item);

    return scope.Close(Undefined());
}

Handle<Value>
sharedHashFile::QueuePushPull(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(3);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_VALIDATE_ARG_IS_INT32(1);
    SHF_VALIDATE_ARG_IS_INT32(2);
    SHF_GET_SHAREDHASHFILE_OBJ();

    uint32_t uid      = arg0;
    uint32_t uid4push = arg1;
    uint32_t uid4pull = arg2;
    if (SHF_UID_NONE == uid) {
        SHF_DEBUG("- uid is SHF_UID_NONE==0x%08x\n", uid);
    }
    else {
        SHF_DEBUG("- uid is 0x%08x\n", uid);
        obj->shf->QueuePushHead(uid4push, uid);
    }

    obj->shf->QueuePullTail(uid4pull);
    if (SHF_UID_NONE == shf_uid) {
        return scope.Close(Undefined());
    }
    else {
        return scope.Close(String::NewExternal(new CustomExternalAsciiStringResource(shf_item_addr, shf_item_addr_len)));
    }
}

Handle<Value>
sharedHashFile::QueuePullTail(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(1);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_GET_SHAREDHASHFILE_OBJ();

    obj->shf->QueuePullTail(arg0);

    if (SHF_UID_NONE == shf_uid)
        return scope.Close(Undefined());

    Local<v8::Array> result = v8::Array::New(2);
    result->Set(0, v8::Integer::New(shf_uid));
    result->Set(1, v8::String::New(SHF_CAST(char *, shf_item_addr), shf_item_addr_len));
    return scope.Close(result);
}

Handle<Value>
sharedHashFile::QueueTakeItem(const Arguments& args) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(2);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_VALIDATE_ARG_IS_INT32(1);
    SHF_GET_SHAREDHASHFILE_OBJ();

    ThrowException(Exception::TypeError(String::New("Function not implemented yet!")));

    obj->shf->QueueTakeItem(arg0, arg1);

    if (SHF_UID_NONE == shf_uid)
        return scope.Close(Undefined());

    Local<v8::Array> result = v8::Array::New(2);
    result->Set(0, v8::Integer::New(shf_uid));
    result->Set(1, v8::String::New(SHF_CAST(char *, shf_item_addr), shf_item_addr_len));
    return scope.Close(result);
}

Handle<Value>
sharedHashFile::Dummy1(const Arguments& args) { /* Example function to show how fast simple C++ functions can be called */
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    return scope.Close(Undefined());
}

Handle<Value>
sharedHashFile::Dummy2(const Arguments& args) { /* Example function to show how fast simple C++ functions can be called */
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_GET_SHAREDHASHFILE_OBJ();
    SHF_UNUSE(obj);
    return scope.Close(Undefined());
}

Handle<Value>
sharedHashFile::Dummy3a(const Arguments& args) { /* Example function to show how fast simple C++ functions can be called */
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(1);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_GET_SHAREDHASHFILE_OBJ();
    SHF_UNUSE(arg0); SHF_UNUSE(obj);
    return scope.Close(Undefined());
}

Handle<Value>
sharedHashFile::Dummy3b(const Arguments& args) { /* Example function to show how fast simple C++ functions can be called */
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(3);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_VALIDATE_ARG_IS_INT32(1);
    SHF_VALIDATE_ARG_IS_INT32(2);
    SHF_GET_SHAREDHASHFILE_OBJ();
    SHF_UNUSE(arg0); SHF_UNUSE(arg1); SHF_UNUSE(arg2); SHF_UNUSE(obj);
    return scope.Close(Undefined());
}

Handle<Value>
sharedHashFile::Dummy4(const Arguments& args) { /* Example function to show how fast simple C++ functions can be called */
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(3);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_VALIDATE_ARG_IS_INT32(1);
    SHF_VALIDATE_ARG_IS_INT32(2);
    SHF_GET_SHAREDHASHFILE_OBJ();
    SHF_UNUSE(arg0); SHF_UNUSE(arg1); SHF_UNUSE(arg2); SHF_UNUSE(obj);
    return scope.Close(Number::New(123));
}

Handle<Value>
sharedHashFile::Dummy5(const Arguments& args) { /* Example function to show how fast simple C++ functions can be called */
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(3);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_VALIDATE_ARG_IS_INT32(1);
    SHF_VALIDATE_ARG_IS_INT32(2);
    SHF_GET_SHAREDHASHFILE_OBJ();
    SHF_UNUSE(arg0); SHF_UNUSE(arg1); SHF_UNUSE(arg2); SHF_UNUSE(obj);
    return scope.Close(String::New(shfDummyBytes, 8));
}

Handle<Value>
sharedHashFile::Dummy6(const Arguments& args) { /* Example function to show how fast simple C++ functions can be called */
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(3);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_VALIDATE_ARG_IS_INT32(1);
    SHF_VALIDATE_ARG_IS_INT32(2);
    SHF_GET_SHAREDHASHFILE_OBJ();
    SHF_UNUSE(arg0); SHF_UNUSE(arg1); SHF_UNUSE(arg2); SHF_UNUSE(obj);
    return scope.Close(String::New(shfDummyBytes, SHF_DUMMY_BYTES));
}

Handle<Value>
sharedHashFile::Dummy7a(const Arguments& args) { /* Example function to show how fast simple C++ functions can be called */
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(3);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_VALIDATE_ARG_IS_INT32(1);
    SHF_VALIDATE_ARG_IS_INT32(2);
    SHF_GET_SHAREDHASHFILE_OBJ();
    SHF_UNUSE(arg0); SHF_UNUSE(arg1); SHF_UNUSE(arg2); SHF_UNUSE(obj);
    return scope.Close(String::NewExternal(new CustomExternalStringResource(shfDummyBytes, SHF_DUMMY_BYTES)));
}

Handle<Value>
sharedHashFile::Dummy7b(const Arguments& args) { /* Example function to show how fast simple C++ functions can be called */
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(3);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_VALIDATE_ARG_IS_INT32(1);
    SHF_VALIDATE_ARG_IS_INT32(2);
    SHF_GET_SHAREDHASHFILE_OBJ();
    SHF_UNUSE(arg0); SHF_UNUSE(arg1); SHF_UNUSE(arg2); SHF_UNUSE(obj);
    Local<v8::Array> result = v8::Array::New(1);
    result->Set(0, String::NewExternal(new CustomExternalStringResource(shfDummyBytes, SHF_DUMMY_BYTES)));
    return scope.Close(result);
}

Handle<Value>
sharedHashFile::Dummy7c(const Arguments& args) { /* Example function to show how fast simple C++ functions can be called */
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(3);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_VALIDATE_ARG_IS_INT32(1);
    SHF_VALIDATE_ARG_IS_INT32(2);
    SHF_GET_SHAREDHASHFILE_OBJ();
    SHF_UNUSE(arg0); SHF_UNUSE(arg1); SHF_UNUSE(arg2); SHF_UNUSE(obj);
    Local<v8::Array> result = v8::Array::New(10);
    result->Set(0, String::NewExternal(new CustomExternalStringResource(shfDummyBytes, SHF_DUMMY_BYTES)));
    result->Set(1, String::NewExternal(new CustomExternalStringResource(shfDummyBytes, SHF_DUMMY_BYTES)));
    result->Set(2, String::NewExternal(new CustomExternalStringResource(shfDummyBytes, SHF_DUMMY_BYTES)));
    result->Set(3, String::NewExternal(new CustomExternalStringResource(shfDummyBytes, SHF_DUMMY_BYTES)));
    result->Set(4, String::NewExternal(new CustomExternalStringResource(shfDummyBytes, SHF_DUMMY_BYTES)));
    result->Set(5, String::NewExternal(new CustomExternalStringResource(shfDummyBytes, SHF_DUMMY_BYTES)));
    result->Set(6, String::NewExternal(new CustomExternalStringResource(shfDummyBytes, SHF_DUMMY_BYTES)));
    result->Set(7, String::NewExternal(new CustomExternalStringResource(shfDummyBytes, SHF_DUMMY_BYTES)));
    result->Set(8, String::NewExternal(new CustomExternalStringResource(shfDummyBytes, SHF_DUMMY_BYTES)));
    result->Set(9, String::NewExternal(new CustomExternalStringResource(shfDummyBytes, SHF_DUMMY_BYTES)));
    return scope.Close(result);
}

Handle<Value>
sharedHashFile::Dummy8(const Arguments& args) { /* Example function to show how fast simple C++ functions can be called */
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(3);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_VALIDATE_ARG_IS_INT32(1);
    SHF_VALIDATE_ARG_IS_INT32(2);
    SHF_GET_SHAREDHASHFILE_OBJ();
    SHF_UNUSE(arg0); SHF_UNUSE(arg1); SHF_UNUSE(arg2); SHF_UNUSE(obj);
    return scope.Close(Buffer::New(shfDummyBytes, SHF_DUMMY_BYTES)->handle_); /* ->handle_? see https://groups.google.com/d/msg/nodejs/yHXD-z5nfoI/jvYLuahEqrEJ */
}

Handle<Value>
sharedHashFile::Dummy9(const Arguments& args) { /* Example function to show how fast simple C++ functions can be called */
    SHF_DEBUG("%s()\n", __FUNCTION__);
    SHF_HANDLE_SCOPE();
    SHF_VALIDATE_ARG_COUNT_REQUIRED(3);
    SHF_VALIDATE_ARG_IS_INT32(0);
    SHF_VALIDATE_ARG_IS_INT32(1);
    SHF_VALIDATE_ARG_IS_INT32(2);
    SHF_GET_SHAREDHASHFILE_OBJ();
    SHF_UNUSE(arg0); SHF_UNUSE(arg1); SHF_UNUSE(arg2); SHF_UNUSE(obj);
    return scope.Close(Buffer::New(shfDummyBytes, SHF_DUMMY_BYTES, [](char*, void*) -> void { /* Don't need to do anything here, because the data is shared memory */ }, NULL)->handle_);
    //todo why is this zero-copy Buffer::New() soooo slow?
}

void
InitAll(Handle<Object> exports) {
    SHF_DEBUG("%s()\n", __FUNCTION__);
    sharedHashFile::Init(exports);
}

NODE_MODULE(SharedHashFile, InitAll)