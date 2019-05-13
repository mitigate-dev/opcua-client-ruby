module OPCUAClient
  class Client
    def after_session_created(&block)
      @callback_after_session_created = block
    end

    def after_data_changed(&block)
      @callback_after_data_changed = block
    end

    def human_state
      state = self.state

      if state == OPCUAClient::UA_CLIENTSTATE_DISCONNECTED; "UA_CLIENTSTATE_DISCONNECTED"
      elsif state == OPCUAClient::UA_CLIENTSTATE_CONNECTED; "UA_CLIENTSTATE_CONNECTED"
      elsif state == OPCUAClient::UA_CLIENTSTATE_SECURECHANNEL; "UA_CLIENTSTATE_SECURECHANNEL"
      elsif state == OPCUAClient::UA_CLIENTSTATE_SESSION; "UA_CLIENTSTATE_SESSION"
      elsif state == OPCUAClient::UA_CLIENTSTATE_SESSION_RENEWED; "UA_CLIENTSTATE_SESSION_RENEWED"
      end
    end
  end
end
