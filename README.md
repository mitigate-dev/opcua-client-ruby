# opcua-client-ruby

Incomplete OPC-UA client library for Ruby. Wraps open62541: <https://open62541.org>.

## Installation

Add it to your Gemfile:

```ruby
gem 'opcua_client'
```

## Basic usage

Use `start` helper to automatically close connections:

```ruby
require 'opcua_client'

OPCUAClient.start("opc.tcp://127.0.0.1:4840") do |client|
  # write to ns=2;s=1
  client.write_int16(2, "1", 888)
  puts client.read_int16(2, "1")
end
```

Or handle connections manually:

```ruby
require 'opcua_client'

client = OPCUAClient::Client.new
begin
  client.connect("opc.tcp://127.0.0.1:4840")
  # write to ns=2;s=1
  client.write_int16(2, "1", 888)
  puts client.read_int16(2, "1")
ensure
  client.disconnect
end
```

## Subscriptions and monitoring

```ruby
cli = OPCUAClient::Client.new

cli.after_session_created do |cli|
  subscription_id = cli.create_subscription
  ns_index = 1
  ns_name = "the.answer"
  cli.add_monitored_item(subscription_id, ns_index, ns_name)
end

cli.after_data_changed do |subscription_id, monitor_id, server_time, source_time, new_value|
  puts("data changed: " + [subscription_id, monitor_id, server_time, source_time, new_value].inspect)
end

cli.connect("opc.tcp://127.0.0.1:4840")

loop do
  cli.connect("opc.tcp://127.0.0.1:4840") # no-op if connected
  cli.run_mon_cycle
  sleep(0.2)
end
```

## Contribute

### Set up

```console
bundle
```

### Try out changes

```console
$ rake compile
$ bin/console
pry> client = OPCUAClient::Client.new
```
