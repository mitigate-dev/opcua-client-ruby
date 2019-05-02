#include <ruby.h>
#include "open62541.h"

VALUE cClient;
VALUE cError;
VALUE mOPCUAClient;

struct UninitializedClient {
    UA_Client *client;
};

struct OpcuaClientContext {
    UA_UInt16 monNsIndex;
    UA_UInt32 monIdentifier;
};

static void
handler_currentTimeChanged(UA_Client *client, UA_UInt32 subId, void *subContext,
                           UA_UInt32 monId, void *monContext, UA_DataValue *value) {
    printf("currentTime has changed!\n");
    if(UA_Variant_hasScalarType(&value->value, &UA_TYPES[UA_TYPES_DATETIME])) {
        UA_DateTime raw_date = *(UA_DateTime *) value->value.data;
        UA_DateTimeStruct dts = UA_DateTime_toStruct(raw_date);
        printf("date is: %02u-%02u-%04u %02u:%02u:%02u.%03u", dts.day, dts.month, dts.year, dts.hour, dts.min, dts.sec, dts.milliSec);
    }
}

static void
deleteSubscriptionCallback(UA_Client *client, UA_UInt32 subscriptionId, void *subscriptionContext) {
    printf("Subscription Id %u was deleted\n", subscriptionId);
}

static void
subscriptionInactivityCallback (UA_Client *client, UA_UInt32 subscriptionId, void *subContext) {
    printf("Inactivity for subscription %u", subscriptionId);
}

static void
stateCallback (UA_Client *client, UA_ClientState clientState) {
    struct OpcuaClientContext *ctx = UA_Client_getContext(client);
    
    switch(clientState) {
        case UA_CLIENTSTATE_DISCONNECTED:
            printf("%s\n", "The client is disconnected");
            break;
        case UA_CLIENTSTATE_CONNECTED:
            printf("%s\n", "A TCP connection to the server is open");
            break;
        case UA_CLIENTSTATE_SECURECHANNEL:
            printf("%s\n", "A SecureChannel to the server is open");
            break;
        case UA_CLIENTSTATE_SESSION:
            printf("%s\n", "A new session was created");
            UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
            UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(client, request, NULL, NULL, deleteSubscriptionCallback);
            
            if (response.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
                printf("Create subscription succeeded, id %u\n", response.subscriptionId);
                
                UA_MonitoredItemCreateRequest monRequest = UA_MonitoredItemCreateRequest_default(UA_NODEID_NUMERIC(ctx->monNsIndex, ctx->monIdentifier));
                
                UA_MonitoredItemCreateResult monResponse =
                UA_Client_MonitoredItems_createDataChange(client, response.subscriptionId,
                                                          UA_TIMESTAMPSTORETURN_BOTH,
                                                          monRequest, NULL, handler_currentTimeChanged, NULL);
                if (monResponse.statusCode == UA_STATUSCODE_GOOD) {
                    printf("Monitoring UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME, id %u\n", monResponse.monitoredItemId);
                }
            }

            break;
        case UA_CLIENTSTATE_SESSION_RENEWED:
            /* The session was renewed. We don't need to recreate the subscription. */
            break;
    }
    return;
}

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
    // UA_Client_delete(self);
    
    struct UninitializedClient *uclient = self;
    
    if (uclient->client) {
        struct OpcuaClientContext *ctx = UA_Client_getContext(uclient->client);
        xfree(ctx);
        
        UA_Client_delete(uclient->client);
    }

    xfree(self);
}

static const rb_data_type_t UA_Client_Type = {
    "UA_Uninitialized_Client",
    { 0, UA_Client_free, 0 },
    0, 0, RUBY_TYPED_FREE_IMMEDIATELY,
};

static VALUE allocate(VALUE klass) {
    struct UninitializedClient *uclient = ALLOC(struct UninitializedClient);
    return TypedData_Wrap_Struct(klass, &UA_Client_Type, uclient);
}

static VALUE rb_initialize(VALUE self) {
    struct UninitializedClient * uclient;
    TypedData_Get_Struct(self, struct UninitializedClient, &UA_Client_Type, uclient);
    
    UA_ClientConfig customConfig = UA_ClientConfig_default;
    customConfig.stateCallback = stateCallback;
    customConfig.subscriptionInactivityCallback = subscriptionInactivityCallback;
    
    struct OpcuaClientContext *ctx = ALLOC(struct OpcuaClientContext);
    // TODO: from ruby
    ctx->monNsIndex = 0;
    ctx->monIdentifier = UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME;

    customConfig.clientContext = ctx;
    
    uclient->client = UA_Client_new(customConfig);
    
    return Qnil;
}

static VALUE rb_connect(VALUE self, VALUE v_connectionString) {
    if (RB_TYPE_P(v_connectionString, T_STRING) != 1) {
        return raise_invalid_arguments_error();
    }
    
    char *connectionString = StringValueCStr(v_connectionString);
    
    struct UninitializedClient * uclient;
    TypedData_Get_Struct(self, struct UninitializedClient, &UA_Client_Type, uclient);
    UA_Client *client = uclient->client;
    
    UA_StatusCode status = UA_Client_connect(client, connectionString);
    
    if (status == UA_STATUSCODE_GOOD) {
        return Qnil;
    } else {
        return raise_ua_status_error(status);
    }
}

static VALUE rb_disconnect(VALUE self) {
    struct UninitializedClient * uclient;
    TypedData_Get_Struct(self, struct UninitializedClient, &UA_Client_Type, uclient);
    UA_Client *client = uclient->client;
    
    UA_StatusCode status = UA_Client_disconnect(client);
    return RB_UINT2NUM(status);
}

static VALUE rb_writeUaValue(VALUE self, VALUE v_nsIndex, VALUE v_name, VALUE v_newValue, int uaType) {
    if (RB_TYPE_P(v_name, T_STRING) != 1) {
        return raise_invalid_arguments_error();
    }
    
    if (RB_TYPE_P(v_nsIndex, T_FIXNUM) != 1) {
        return raise_invalid_arguments_error();
    }
    
    if (uaType == UA_TYPES_INT16 && RB_TYPE_P(v_newValue, T_FIXNUM) != 1) {
        return raise_invalid_arguments_error();
    }
    
    char *name = StringValueCStr(v_name);
    int nsIndex = FIX2INT(v_nsIndex);
    
    struct UninitializedClient * uclient;
    TypedData_Get_Struct(self, struct UninitializedClient, &UA_Client_Type, uclient);
    UA_Client *client = uclient->client;
    
    UA_Variant value;
    UA_Variant_init(&value);
    
    if (uaType == UA_TYPES_INT16) {
        UA_Int16 newValue = NUM2SHORT(v_newValue);
        value.data = UA_malloc(sizeof(UA_Int16));
        *(UA_Int16*)value.data = newValue;
        value.type = &UA_TYPES[UA_TYPES_INT16];
    } else if (uaType == UA_TYPES_INT32) {
        UA_Int32 newValue = NUM2INT(v_newValue);
        value.data = UA_malloc(sizeof(UA_Int32));
        *(UA_Int32*)value.data = newValue;
        value.type = &UA_TYPES[UA_TYPES_INT32];
    } else if (uaType == UA_TYPES_FLOAT) {
        UA_Float newValue = NUM2DBL(v_newValue);
        value.data = UA_malloc(sizeof(UA_Float));
        *(UA_Float*)value.data = newValue;
        value.type = &UA_TYPES[UA_TYPES_FLOAT];
    } else if (uaType == UA_TYPES_BOOLEAN) {
        UA_Boolean newValue = RTEST(v_newValue);
        value.data = UA_malloc(sizeof(UA_Boolean));
        *(UA_Boolean*)value.data = newValue;
        value.type = &UA_TYPES[UA_TYPES_BOOLEAN];
    } else {
        rb_raise(cError, "Unsupported type");
    }

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

static VALUE rb_writeInt16Value(VALUE self, VALUE v_nsIndex, VALUE v_name, VALUE v_newValue) {
    return rb_writeUaValue(self, v_nsIndex, v_name, v_newValue, UA_TYPES_INT16);
}

static VALUE rb_writeInt32Value(VALUE self, VALUE v_nsIndex, VALUE v_name, VALUE v_newValue) {
    return rb_writeUaValue(self, v_nsIndex, v_name, v_newValue, UA_TYPES_INT32);
}

static VALUE rb_writeBooleanValue(VALUE self, VALUE v_nsIndex, VALUE v_name, VALUE v_newValue) {
    return rb_writeUaValue(self, v_nsIndex, v_name, v_newValue, UA_TYPES_BOOLEAN);
}

static VALUE rb_writeFloatValue(VALUE self, VALUE v_nsIndex, VALUE v_name, VALUE v_newValue) {
    return rb_writeUaValue(self, v_nsIndex, v_name, v_newValue, UA_TYPES_FLOAT);
}

static VALUE rb_readUaValue(VALUE self, VALUE v_nsIndex, VALUE v_name, int type) {
    if (RB_TYPE_P(v_name, T_STRING) != 1) {
        return raise_invalid_arguments_error();
    }
    
    if (RB_TYPE_P(v_nsIndex, T_FIXNUM) != 1) {
        return raise_invalid_arguments_error();
    }
    
    char *name = StringValueCStr(v_name);
    int nsIndex = FIX2INT(v_nsIndex);
    
    struct UninitializedClient * uclient;
    TypedData_Get_Struct(self, struct UninitializedClient, &UA_Client_Type, uclient);
    UA_Client *client = uclient->client;
    
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
    
    if (type == UA_TYPES_INT16 && UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT16])) {
        UA_Int16 val =*(UA_Int16*)value.data;
        // printf("the value is: %i\n", val);
        result = INT2FIX(val);
    } else if (type == UA_TYPES_INT32 && UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT32])) {
        UA_Int32 val =*(UA_Int32*)value.data;
        result = INT2FIX(val);
    } else if (type == UA_TYPES_BOOLEAN && UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_BOOLEAN])) {
        UA_Boolean val =*(UA_Boolean*)value.data;
        result = val ? Qtrue : Qfalse;
    } else if (type == UA_TYPES_FLOAT && UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_FLOAT])) {
        UA_Float val =*(UA_Float*)value.data;
        result = DBL2NUM(val);
    } else {
        rb_raise(cError, "Not an int16");
        return Qnil;
    }
    
    /* Clean up */
    UA_Variant_deleteMembers(&value);
    
    return result;
}

static VALUE rb_readInt16Value(VALUE self, VALUE v_nsIndex, VALUE v_name) {
    return rb_readUaValue(self, v_nsIndex, v_name, UA_TYPES_INT16);
}

static VALUE rb_readInt32Value(VALUE self, VALUE v_nsIndex, VALUE v_name) {
    return rb_readUaValue(self, v_nsIndex, v_name, UA_TYPES_INT32);
}

static VALUE rb_readBooleanValue(VALUE self, VALUE v_nsIndex, VALUE v_name) {
    return rb_readUaValue(self, v_nsIndex, v_name, UA_TYPES_BOOLEAN);
}

static VALUE rb_readFloatValue(VALUE self, VALUE v_nsIndex, VALUE v_name) {
    return rb_readUaValue(self, v_nsIndex, v_name, UA_TYPES_FLOAT);
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

VALUE rb_run_single_monitoring_cycle(VALUE self) {
    struct UninitializedClient * uclient;
    TypedData_Get_Struct(self, struct UninitializedClient, &UA_Client_Type, uclient);
    UA_Client *client = uclient->client;
    
    UA_StatusCode status = UA_Client_runAsync(client, 1000);
    return UINT2NUM(status);
}

VALUE rb_run_single_monitoring_cycle_bang(VALUE self) {
    struct UninitializedClient * uclient;
    TypedData_Get_Struct(self, struct UninitializedClient, &UA_Client_Type, uclient);
    UA_Client *client = uclient->client;
    
    UA_StatusCode status = UA_Client_runAsync(client, 1000);
    
    if (status != UA_STATUSCODE_GOOD) {
        return raise_ua_status_error(status);
    }
    
    return Qnil;
}

void Init_opcua_client()
{
#ifdef UA_ENABLE_SUBSCRIPTIONS
    printf("%s\n", "ok! opcua-client-ruby built with subscriptions enabled.");
#endif
    
    mOPCUAClient = rb_const_get(rb_cObject, rb_intern("OPCUAClient"));
    cError = rb_define_class_under(mOPCUAClient, "Error", rb_eStandardError);
    cClient = rb_define_class_under(mOPCUAClient, "Client", rb_cObject);
    
    rb_define_alloc_func(cClient, allocate);
    
    rb_define_method(cClient, "initialize", rb_initialize, 0);
    rb_define_method(cClient, "run_single_monitoring_cycle", rb_run_single_monitoring_cycle, 0);
    rb_define_method(cClient, "run_mon_cycle", rb_run_single_monitoring_cycle, 0);
    rb_define_method(cClient, "do_mon_cycle", rb_run_single_monitoring_cycle, 0);
    
    rb_define_method(cClient, "run_single_monitoring_cycle!", rb_run_single_monitoring_cycle_bang, 0);
    rb_define_method(cClient, "run_mon_cycle!", rb_run_single_monitoring_cycle_bang, 0);
    rb_define_method(cClient, "do_mon_cycle!", rb_run_single_monitoring_cycle_bang, 0);
    
    rb_define_method(cClient, "connect", rb_connect, 1);
    rb_define_method(cClient, "disconnect", rb_disconnect, 0);
    
    rb_define_method(cClient, "read_int16", rb_readInt16Value, 2);
    rb_define_method(cClient, "read_int32", rb_readInt32Value, 2);
    rb_define_method(cClient, "read_float", rb_readFloatValue, 2);
    rb_define_method(cClient, "read_boolean", rb_readBooleanValue, 2);
    rb_define_method(cClient, "read_bool", rb_readBooleanValue, 2);
    
    rb_define_method(cClient, "write_int16", rb_writeInt16Value, 3);
    rb_define_method(cClient, "write_int32", rb_writeInt32Value, 3);
    rb_define_method(cClient, "write_float", rb_writeFloatValue, 3);
    rb_define_method(cClient, "write_boolean", rb_writeBooleanValue, 3);
    rb_define_method(cClient, "write_bool", rb_writeBooleanValue, 3);
    
    rb_define_singleton_method(mOPCUAClient, "human_status_code", rb_get_human_UA_StatusCode, 1);
}
