AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = -g -D_XOPEN_SOURCE=600 -D_BSD_SOURCE=1 -D_THREAD_SAFE @LIB_OS_SPECIFIC_F@
AM_LDFLAGS = -g -o @LIB_OS_SPECIFIC_L@
INCLUDES =  -I./ @LIB_OS_SPECIFIC_I@

include_HEADERS = chars.hpp charset.hpp cleargif.hpp condsync.hpp crc32.hpp dams.hpp datamanagerapi.hpp \
	detectcharset.hpp dfile.hpp dir.hpp ehttpthread.hpp esendthread.hpp eventpoller.hpp exception.hpp file.hpp \
	file2.hpp hash.hpp hashmap.hpp http.hpp httpclient.hpp httpevents.hpp httpkeepalive.hpp httprequest.hpp \
	httpresponse.hpp httpsevent.hpp httpsocket.hpp icharset.hpp ips.hpp key.hpp limit.hpp log.hpp \
	mail_smtp.hpp md5.hpp mutexsync.hpp mutexthrman.hpp mysql.hpp mysqlsync.hpp operaturbo.hpp punycode.hpp \
	rand.hpp resolv.hpp rwsync.hpp sendevent.hpp seqkoi.hpp smartptr.hpp socket.hpp socket_raw.hpp socket_ssl.hpp \
	sort.hpp spechttp.hpp spf.hpp srt_array.hpp sslexcache.hpp stopwatch.hpp storage.hpp storageman.hpp string.hpp \
	thirdleveldomains.hpp thread.hpp time.hpp tools.hpp tr1.hpp TSTPtr.hpp vararray.hpp xml.hpp xmlhash.hpp xtypes.hpp

EXTRA_DIST = rapidjson rapidxml consts certificates httpclear.gif

noinst_LIBRARIES = libchars.a libcharset.a libcleargif.a libcrc32.a libdatamanagerapi.a libdetectcharset.a libdfile.a \
	libdir.a libehttpthread.a libesendthread.a libeventpoller.a libfile.a libfile2.a libhttp.a libhttpclient.a libhttpevents.a \
	libhttpkeepalive.a libhttprequest.a libhttpsevent.a libhttpsocket.a libicharset.a libips.a liblimit.a liblog.a \
	libmail_smtp.a libmd5.a libmutexsync.a libmysql.a libpunycode.a librand.a libresolv.a librwsync.a libsendevent.a libseqkoi.a \
	libsocket.a libsocket_raw.a libsocket_ssl.a libspf.a libsslexcache.a libstorage.a libstorageman.a libstring.a \
	libthirdleveldomains.a libthread.a libtime.a libtools.a	libvararray.a libxml.a

libchars_a_SOURCES = chars.cpp
libcharset_a_SOURCES = charset.cpp
libcleargif_a_SOURCES = cleargif.cpp
libcrc32_a_SOURCES = crc32.cpp
libdatamanagerapi_a_SOURCES = datamanagerapi.cpp
libdetectcharset_a_SOURCES = detectcharset.cpp
libdfile_a_SOURCES = dfile.cpp
libdir_a_SOURCES = dir.cpp
libehttpthread_a_SOURCES = ehttpthread.cpp
libesendthread_a_SOURCES = esendthread.cpp
libeventpoller_a_SOURCES = eventpoller.cpp
libfile_a_SOURCES = file.cpp
libfile2_a_SOURCES = file2.cpp
libhttp_a_SOURCES = http.cpp
libhttpclient_a_SOURCES = httpclient.cpp
libhttpevents_a_SOURCES = httpevents.cpp
libhttpkeepalive_a_SOURCES = httpkeepalive.cpp
libhttprequest_a_SOURCES = httprequest.cpp
libhttpsevent_a_SOURCES = httpsevent.cpp
libhttpsocket_a_SOURCES = httpsocket.cpp
libicharset_a_SOURCES = icharset.cpp
libips_a_SOURCES = ips.cpp
liblimit_a_SOURCES = limit.cpp
liblog_a_SOURCES = log.cpp
libmail_smtp_a_SOURCES = mail_smtp.cpp
libmd5_a_SOURCES = md5.cpp
libmutexsync_a_SOURCES = mutexsync.cpp
libmysql_a_SOURCES = mysql.cpp
libpunycode_a_SOURCES = punycode.cpp
librand_a_SOURCES = rand.cpp
libresolv_a_SOURCES = resolv.cpp
librwsync_a_SOURCES = rwsync.cpp
libsendevent_a_SOURCES = sendevent.cpp
libseqkoi_a_SOURCES = seqkoi.cpp
libsocket_a_SOURCES = socket.cpp
libsocket_raw_a_SOURCES = socket_raw.cpp
libsocket_ssl_a_SOURCES = socket_ssl.cpp
libspf_a_SOURCES = spf.cpp
libsslexcache_a_SOURCES = sslexcache.cpp
libstorage_a_SOURCES = storage.cpp
libstorageman_a_SOURCES = storageman.cpp
libstring_a_SOURCES = string.cpp chars.cpp
libthirdleveldomains_a_SOURCES = thirdleveldomains.cpp
libthread_a_SOURCES = thread.cpp
libtime_a_SOURCES = time.cpp
libtools_a_SOURCES = tools.cpp
libvararray_a_SOURCES = vararray.cpp
libxml_a_SOURCES = xml.cpp
