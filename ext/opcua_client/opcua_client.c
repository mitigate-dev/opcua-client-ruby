#include <ruby.h>
#include "open62541.h"

VALUE cClient;
VALUE cError;
VALUE mOPCUAClient;

VALUE raise_invalid_arguments_error() {
    rb_raise(cError, "Invalid arguments");
    return Qnil;
}

VALUE raise_ua_status_error(UA_StatusCode status) {
    rb_raise(cError, "%u: %s", status, UA_StatusCode_name(status));
    return Qnil;
}

static void UA_Client_free(void *self) {
    // printf("%s\n", "calling UA_Client_delete");
    UA_Client_delete(self);
}

static const rb_data_type_t UA_Client_Type = {
    "UA_Client",
    { 0, UA_Client_free, 0 },
    0, 0, RUBY_TYPED_FREE_IMMEDIATELY,
};

static VALUE allocate(VALUE klass) {
    UA_Client *client = UA_Client_new(UA_ClientConfig_default);
    return TypedData_Wrap_Struct(klass, &UA_Client_Type, client);
}

static VALUE rb_connect(VALUE self, VALUE v_connectionString) {
    if (RB_TYPE_P(v_connectionString, T_STRING) != 1) {
        return raise_invalid_arguments_error();
    }
    
    char *connectionString = StringValueCStr(v_connectionString);
    
    UA_Client *client;
    TypedData_Get_Struct(self, UA_Client, &UA_Client_Type, client);
    
    UA_StatusCode status = UA_Client_connect(client, connectionString);
    
    if (status == UA_STATUSCODE_GOOD) {
        return Qnil;
    } else {
        return raise_ua_status_error(status);
    }
}

static VALUE rb_disconnect(VALUE self) {
    UA_Client *client;
    TypedData_Get_Struct(self, UA_Client, &UA_Client_Type, client);
    UA_StatusCode status = UA_Client_disconnect(client);
    return RB_UINT2NUM(status);
}

static VALUE rb_writeInt16Value(VALUE self, VALUE v_nsIndex, VALUE v_name, VALUE v_newValue) {
    if (RB_TYPE_P(v_name, T_STRING) != 1) {
        return raise_invalid_arguments_error();
    }
    
    if (RB_TYPE_P(v_nsIndex, T_FIXNUM) != 1) {
        return raise_invalid_arguments_error();
    }
    
    if (RB_TYPE_P(v_newValue, T_FIXNUM) != 1) {
        return raise_invalid_arguments_error();
    }
    
    char *name = StringValueCStr(v_name);
    int nsIndex = FIX2INT(v_nsIndex);
    UA_Int16 newValue = FIX2INT(v_newValue);
    
    UA_Client *client;
    TypedData_Get_Struct(self, UA_Client, &UA_Client_Type, client);
    
    UA_Variant value;
    UA_Variant_init(&value);
    
    value.data = UA_malloc(sizeof(UA_Int16));
    *(UA_Int16*)value.data = newValue;
    value.type = &UA_TYPES[UA_TYPES_INT16];
    
    UA_StatusCode status = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(nsIndex, name), &value);
    
    if (status == UA_STATUSCODE_GOOD) {
        // printf("%s\n", "value write successful");
    } else {
        /* Clean up */
        UA_Variant_deleteMembers(&value);
        return raise_ua_status_error(status);
    }
    
    /* Clean up */
    UA_Variant_deleteMembers(&value);
    
    return Qnil;
}

static VALUE rb_readInt16Value(VALUE self, VALUE v_nsIndex, VALUE v_name) {
    if (RB_TYPE_P(v_name, T_STRING) != 1) {
        return raise_invalid_arguments_error();
    }
    
    if (RB_TYPE_P(v_nsIndex, T_FIXNUM) != 1) {
        return raise_invalid_arguments_error();
    }
    
    char *name = StringValueCStr(v_name);
    int nsIndex = FIX2INT(v_nsIndex);
    
    UA_Client *client;
    TypedData_Get_Struct(self, UA_Client, &UA_Client_Type, client);
    
    UA_Variant value;
    UA_Variant_init(&value);
    UA_StatusCode status = UA_Client_readValueAttribute(client, UA_NODEID_STRING(nsIndex, name), &value);
    
    if (status == UA_STATUSCODE_GOOD) {
        // printf("%s\n", "value read successful");
    } else {
        /* Clean up */
        UA_Variant_deleteMembers(&value);
        return raise_ua_status_error(status);
    }
    
    VALUE result = Qnil;
    
    if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT16])) {
        UA_Int16 val =*(UA_Int16*)value.data;
        // printf("the value is: %i\n", val);
        result = INT2FIX(val);
    } else {
        rb_raise(cError, "Not an int16");
        return Qnil;
    }
    
    /* Clean up */
    UA_Variant_deleteMembers(&value);
    
    return result;
}

VALUE rb_get_human_UA_StatusCode(VALUE self, VALUE v_code) {
    if (RB_TYPE_P(v_code, T_FIXNUM) == 1) {
        unsigned int code = FIX2UINT(v_code);
        const char* name = UA_StatusCode_name(code);
        return rb_str_export_locale(rb_str_new_cstr(name));
    } else {
        return raise_invalid_arguments_error();
    }
}

void Init_opcua_client()
{
    mOPCUAClient = rb_const_get(rb_cObject, rb_intern("OPCUAClient"));
    cError = rb_define_class_under(mOPCUAClient, "Error", rb_eStandardError);
    cClient = rb_define_class_under(mOPCUAClient, "Client", rb_cObject);
    
    rb_define_alloc_func(cClient, allocate);
    rb_define_method(cClient, "connect", rb_connect, 1);
    rb_define_method(cClient, "disconnect", rb_disconnect, 0);
    rb_define_method(cClient, "read_int16", rb_readInt16Value, 2);
    rb_define_method(cClient, "write_int16", rb_writeInt16Value, 3);
    
    rb_define_singleton_method(mOPCUAClient, "human_status_code", rb_get_human_UA_StatusCode, 1);
}
