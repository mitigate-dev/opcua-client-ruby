module OPCUAClient
  class Client
    def after_session_created(&block)
      @callback_after_session_created = block
    end

    def after_data_changed(&block)
      @callback_after_data_changed = block
    end
  end
end
