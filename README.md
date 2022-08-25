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

  client.multi_write_int16(2, (1..10).map{|x| "action_#{x}"}, (1..10).map{|x| x * 10}) # 10x writes
  client.multi_write_int32(2, (1..10).map{|x| "amount_#{x}"}, (1..10).map{|x| x * 10 + 1}) # 10x writes
ensure
  client.disconnect
end
```

### Available methods - connection:

* ```client.connect(String url)``` - raises OPCUAClient::Error if unsuccessful
* ```client.disconnect => Fixnum``` - returns status

### Available methods - reads and writes:

All methods raise OPCUAClient::Error if unsuccessful.

* ```client.read_int16(Fixnum ns, String name) => Fixnum```
* ```client.read_uint16(Fixnum ns, String name) => Fixnum```
* ```client.read_int32(Fixnum ns, String name) => Fixnum```
* ```client.read_uint32(Fixnum ns, String name) => Fixnum```
* ```client.read_float(Fixnum ns, String name) => Float```
* ```client.read_boolean(Fixnum ns, String name) => true/false```
* ```client.write_int16(Fixnum ns, String name, Fixnum value)```
* ```client.write_uint16(Fixnum ns, String name, Fixnum value)```
* ```client.write_int32(Fixnum ns, String name, Fixnum value)```
* ```client.write_uint32(Fixnum ns, String name, Fixnum value)```
* ```client.write_float(Fixnum ns, String name, Float value)```
* ```client.write_boolean(Fixnum ns, String name, bool value)```
* ```client.multi_write_int16(Fixnum ns, Array[String] names, Array[Fixnum] values)```
* ```client.multi_write_uint16(Fixnum ns, Array[String] names, Array[Fixnum] values)```
* ```client.multi_write_int32(Fixnum ns, Array[String] names, Array[Fixnum] values)```
* ```client.multi_write_uint32(Fixnum ns, Array[String] names, Array[Fixnum] values)```
* ```client.multi_write_float(Fixnum ns, Array[String] names, Array[Float] values)```
* ```client.multi_write_boolean(Fixnum ns, Array[String] names, Array[bool] values)```

### Available methods - misc:

* ```client.state => Fixnum``` - client internal state
* ```client.human_state => String``` - human readable client internal state
* ```OPCUAClient::Client.human_status_code(Fixnum status) => String``` - returns human status for status

## Subscriptions and monitoring

```ruby
cli = OPCUAClient::Client.new

cli.after_session_created do |cli|
  subscription_id = cli.create_subscription
  ns_index = 1
  node_name = "the.answer"
  cli.add_monitored_item(subscription_id, ns_index, node_name)
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

### Available methods:

* ```client.create_subscription => Fixnum``` - nil if error
* ```client.add_monitored_item(Fixnum subscription, Fixnum ns, String name) => Fixnum``` - nil if error
* ```client.run_mon_cycle``` - returns status
* ```client.run_mon_cycle!``` - raises OPCUAClient::Error if unsuccessful

### Available callbacks:
* ```after_session_created```
* ```after_data_changed```

## Contribute

### Set up

```console
bundle
```

### Try out changes

```console
$ bin/rake compile
$ bin/console
pry> client = OPCUAClient::Client.new
pry> client.connect("opc.tcp://127.0.0.1:4840")
```

### Test it

```console
$ bin/rake compile
$ bin/rake spec
```
