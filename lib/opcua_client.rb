module OPCUAClient
  class << self
    def start(*args)
      client = OPCUAClient::Client.new
      client.connect(*args)
      yield client
    ensure
      client.disconnect
    end
  end
end

require "opcua_client/opcua_client"
