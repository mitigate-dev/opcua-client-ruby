#include <signal.h>
#include <stdio.h>
#include "open62541.h"

static char* newString() {
    // TODO: memory management
    return new char [100];
}

static UA_NodeId addVariableUnder(UA_Server *server, UA_Int16 nsId, int type, const char *desc, const char *name, const char *nodeIdString, const char *qnString, UA_NodeId parentNodeId, void *defaultValue) {

    UA_NodeId referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId typeDefinition = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_VariableAttributes attr = UA_VariableAttributes_default;

    if (type == UA_TYPES_INT32) {
        UA_Int32 initialValue = *(UA_Int32*)defaultValue;
        UA_Variant_setScalar(&attr.value, &initialValue, &UA_TYPES[type]);
    } else if (type == UA_TYPES_INT16) {
        UA_Int16 initialValue = *(UA_Int16*)defaultValue;
        UA_Variant_setScalar(&attr.value, &initialValue, &UA_TYPES[type]);
    } else if (type == UA_TYPES_UINT16) {
        UA_UInt16 initialValue = *(UA_UInt16*)defaultValue;
        UA_Variant_setScalar(&attr.value, &initialValue, &UA_TYPES[type]);
    } else if (type == UA_TYPES_UINT32) {
        UA_UInt32 initialValue = *(UA_UInt32*)defaultValue;
        UA_Variant_setScalar(&attr.value, &initialValue, &UA_TYPES[type]);
    } else if (type == UA_TYPES_BOOLEAN) {
        UA_Boolean initialValue = *(UA_Boolean*)defaultValue;
        UA_Variant_setScalar(&attr.value, &initialValue, &UA_TYPES[type]);
    } else {
        throw "type not supported";
    }

    attr.description = UA_LOCALIZEDTEXT((char*) "en-US", (char*) desc);
    attr.displayName = UA_LOCALIZEDTEXT((char*) "en-US", (char*) name);
    attr.dataType = UA_TYPES[type].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_NodeId nodeId = UA_NODEID_STRING(nsId, (char*) nodeIdString); // node id
    UA_QualifiedName qualifiedName = UA_QUALIFIEDNAME(nsId, (char*) qnString); // browse name
    UA_Server_addVariableNode(server, nodeId, parentNodeId,
                              referenceTypeId, qualifiedName,
                              typeDefinition, attr, NULL, NULL);

    return nodeId;
}

static UA_NodeId addVariable(UA_Server *server, UA_Int16 nsId, int type, const char *desc, const char *name, const char *nodeIdString, const char *qnString, void *defaultValue) {
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    return addVariableUnder(server, nsId, type, desc, name, nodeIdString, qnString, parentNodeId, defaultValue);
}

static void addVariableV2(UA_Server *server, UA_Int16 nsId, int type, const char *variable, UA_Int32 defaultValue = 0) {
    char* varName = newString();
    sprintf(varName, "%s", variable);

    char* desc = newString();
    char* displayName = newString();
    const char* qn = varName;

    sprintf(desc, "%s.desc", varName);
    sprintf(displayName, "%s.dn", varName);

    char* nodeId = newString();
    sprintf(nodeId, "%s", varName);

    UA_NodeId parentNode = addVariable(server, nsId, type, desc, displayName, nodeId, varName, &defaultValue);
}

UA_Boolean running = true;
static void signalHandler(int signum) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Signal received: %i", signum);
    running = false;
}

static void addVariables(UA_Server *server) {
    UA_Int16 ns2Id = UA_Server_addNamespace(server, "ns2"); // id=2
    UA_Int16 ns3Id = UA_Server_addNamespace(server, "ns3"); // id=3
    UA_Int16 ns4Id = UA_Server_addNamespace(server, "ns4"); // id=4
    UA_Int16 ns5Id = UA_Server_addNamespace(server, "ns5"); // id=5

    addVariableV2(server, ns5Id, UA_TYPES_UINT32, "uint32_a");
    addVariableV2(server, ns5Id, UA_TYPES_UINT32, "uint32_b", 1000);
    addVariableV2(server, ns5Id, UA_TYPES_UINT32, "uint32_c", 2000);
    addVariableV2(server, ns5Id, UA_TYPES_UINT16, "uint16_a");
    addVariableV2(server, ns5Id, UA_TYPES_UINT16, "uint16_b", 100);
    addVariableV2(server, ns5Id, UA_TYPES_UINT16, "uint16_c", 200);
    addVariableV2(server, ns5Id, UA_TYPES_BOOLEAN, "bool_a", false);
    addVariableV2(server, ns5Id, UA_TYPES_BOOLEAN, "bool_true", true);
    addVariableV2(server, ns5Id, UA_TYPES_BOOLEAN, "bool_false", false);
}

int main(void) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    UA_ServerConfig *config = UA_ServerConfig_new_default();
    UA_Server *server = UA_Server_new(config);
    addVariables(server);

    UA_StatusCode retval = UA_Server_run(server, &running);

    UA_Server_delete(server);
    UA_ServerConfig_delete(config);
    return (int)retval;
}
