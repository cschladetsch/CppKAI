# Network System

This library relies on Core, and provides Agents and Proxies.

It allows the application programmer to write natural code that is really deeply network dependant - without using callbacks and without the need for explicit threading.

For example:

	// connect to another node using default port
	Node local(nodeIP);
	Proxy<Foo> foo = local.MakeProxy<Foo>(NetworkHandle(1234)); // how you get the handle is up to you
	Future<int> result = foo->TimesTwo(5);						// invoke remote method
	result.arrived += []{ int n } => { cout << ConstDeref<int>(result.Value); }	// do something when it arrives


## Agents

An agent responds to requests.

## Proxy

A proxy is the local reqpresentation of a remote agent. We invoke methods on, access properties of, and use events within Proxies as if they were agents. All the results are **Future** values.

## Domain

A Domain is a network-aware KAI Registry. Objects within a Domain have a unique NetworkHandle. This is synonymous with objects in a Registry having a unique Handle.

## System Agnostic

There is no expsosure to any underlying networking sub-system.

The first implementation will use RakNet. However, none of the exposed types use Raknet and you will not need Raknet headers to compile. However of course you will need the libraries to link.
