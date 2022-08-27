require 'spec_helper'

RSpec.describe "Reads and writes" do
  let(:client) { new_client }

  let(:max_uint32) { 4_294_967_295 }

  it "reads stored bools" do
    expect(client.read_bool(5, "bool_true")).to eq(true)
    expect(client.read_bool(5, "bool_false")).to eq(false)
  end

  it "reads and writes bools" do
    client.write_bool(5, "bool_a", false)
    expect(client.read_bool(5, "bool_a")).to eq(false)

    client.write_bool(5, "bool_a", true)
    expect(client.read_bool(5, "bool_a")).to eq(true)
  end

  it "reads stored uint32" do
    expect(client.read_uint32(5, "uint32_b")).to eq(1000)
  end

  it "reads and writes uint32" do
    client.write_uint32(5, "uint32_a", max_uint32)
    expect(client.read_uint32(5, "uint32_a")).to eq(max_uint32)

    client.write_uint32(5, "uint32_a", -1)
    expect(client.read_uint32(5, "uint32_a")).to eq(max_uint32)

    client.write_uint32(5, "uint32_a", 0)
    expect(client.read_uint32(5, "uint32_a")).to eq(0)
  end
end
