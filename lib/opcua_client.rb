module OPCUAClient
  class << self
    def new_client
      OPCUAClient::Client.new
    end

    def start(*args)
      client = OPCUAClient::Client.new
      client.connect(*args)
      yield client
    ensure
      client.disconnect
    end
  end
end

require "opcua_client/client"
