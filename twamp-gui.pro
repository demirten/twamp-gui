TEMPLATE = subdirs

SUBDIRS = common client responder

client.depends = common
responder.depends = common
