TEMPLATE = subdirs

SUBDIRS = common client responder

client.depends = common
responder.depends = common

# build the project sequentially as listed in SUBDIRS !
CONFIG += ordered
