noinst_LTLIBRARIES = librendermq_test_common.la

check_PROGRAMS = \
	test_consistent_hash \
	test_disk_storage \
	test_handler \
	test_mongrel_request_parser \
	test_per_style_storage \
	test_priority_queue \
	test_style_rules \
	test_union_storage \
	test_zmq_queue \
	test_zstream

#	test_expiry_overlay \
#	test_lts_storage \
#	test_mdots

test_consistent_hash_SOURCES = \
	test_consistent_hash.cpp
test_consistent_hash_CPPFLAGS = $(DEPS_CFLAGS) $(BOOST_CPPFLAGS)
test_consistent_hash_LDADD = \
	../librendermq_logging.la \
	../librendermq_dqueue.la \
	librendermq_test_common.la \
	$(DEPS_LIBS) $(BOOST_LIBS)

test_disk_storage_SOURCES = \
	test_disk_storage.cpp
test_disk_storage_CPPFLAGS = $(DEPS_CFLAGS) $(BOOST_CPPFLAGS)
test_disk_storage_LDADD = \
	../librendermq_logging.la \
	../librendermq_storage.la \
	librendermq_test_common.la \
	$(DEPS_LIBS) $(BOOST_LIBS)

test_handler_SOURCES = \
	test_handler.cpp \
	../tile_path_parser.cpp
test_handler_CPPFLAGS = $(DEPS_CFLAGS) $(BOOST_CPPFLAGS)
test_handler_LDADD = \
	../librendermq_logging.la \
	../librendermq_http.la \
	../librendermq_proto.la \
	librendermq_test_common.la \
	$(DEPS_LIBS) $(BOOST_LIBS)

test_mongrel_request_parser_SOURCES = \
	test_mongrel_request_parser.cpp \
	../mongrel_request.cpp \
	../mongrel_request_parser.cpp
test_mongrel_request_parser_CPPFLAGS = $(DEPS_CFLAGS) $(BOOST_CPPFLAGS)
test_mongrel_request_parser_LDADD = \
	../librendermq_logging.la \
	../librendermq_http.la \
	librendermq_test_common.la \
	$(DEPS_LIBS) $(BOOST_LIBS)

test_per_style_storage_SOURCES = \
	test_per_style_storage.cpp
test_per_style_storage_CPPFLAGS = $(DEPS_CFLAGS) $(BOOST_CPPFLAGS)
test_per_style_storage_LDADD = \
	../librendermq_logging.la \
	../librendermq_storage.la \
	../librendermq_proto.la \
	librendermq_test_common.la \
	$(DEPS_LIBS) $(BOOST_LIBS)

test_priority_queue_SOURCES = \
	test_priority_queue.cpp
test_priority_queue_CPPFLAGS = $(DEPS_CFLAGS) $(BOOST_CPPFLAGS)
test_priority_queue_LDADD = \
	../librendermq_logging.la \
	../librendermq_proto.la \
	librendermq_test_common.la \
	$(DEPS_LIBS) $(BOOST_LIBS)

test_style_rules_SOURCES = \
	test_style_rules.cpp \
	../mongrel_request.cpp \
	../tile_path_parser.cpp \
	../mongrel_request_parser.cpp \
	../storage_worker.cpp \
	../tile_handler.cpp
test_style_rules_CPPFLAGS = $(DEPS_CFLAGS) $(BOOST_CPPFLAGS)
test_style_rules_LDADD = \
	../librendermq_logging.la \
	../librendermq_proto.la \
	../librendermq_dqueue.la \
	../librendermq_http.la \
	../librendermq_storage.la \
	librendermq_test_common.la \
	$(DEPS_LIBS) $(BOOST_LIBS)

test_union_storage_SOURCES = \
	test_union_storage.cpp
test_union_storage_CPPFLAGS = $(DEPS_CFLAGS) $(BOOST_CPPFLAGS)
test_union_storage_LDADD = \
	../librendermq_logging.la \
	../librendermq_storage.la \
	../librendermq_proto.la \
	librendermq_test_common.la \
	$(DEPS_LIBS) $(BOOST_LIBS)

test_zmq_queue_SOURCES = \
	test_zmq_queue.cpp \
	../tile_broker_impl.cpp
test_zmq_queue_CPPFLAGS = $(DEPS_CFLAGS) $(BOOST_CPPFLAGS)
test_zmq_queue_LDADD = \
	../librendermq_logging.la \
	../librendermq_proto.la \
	../librendermq_dqueue.la \
	librendermq_test_common.la \
	$(DEPS_LIBS) $(BOOST_LIBS)

test_zstream_SOURCES = \
	test_zstream.cpp
test_zstream_CPPFLAGS = $(DEPS_CFLAGS) $(BOOST_CPPFLAGS)
test_zstream_LDADD = \
	../librendermq_logging.la \
	../librendermq_proto.la \
	librendermq_test_common.la \
	$(DEPS_LIBS) $(BOOST_LIBS)

librendermq_test_common_la_SOURCES = \
	common.cpp \
	fake_tile.cpp
librendermq_test_common_la_CPPFLAGS = $(DEPS_CFLAGS) $(BOOST_CPPFLAGS)
librendermq_test_common_la_LIBADD = \
	../librendermq_logging.la \
	$(DEPS_LIBS) $(BOOST_LIBS)

TESTS = $(check_PROGRAMS)
