idf_build_get_property(target IDF_TARGET)

set(srcs
    "exosip/eXcall_api.c"
    "exosip/eXconf.c"
    "exosip/eXinsubscription_api.c"
    "exosip/eXmessage_api.c"
    "exosip/eXoptions_api.c"
    "exosip/eXosip.c"
    "exosip/eXpublish_api.c"
    "exosip/eXrefer_api.c"
    "exosip/eXregister_api.c"
    "exosip/eXsubscription_api.c"
    "exosip/eXtl_dtls.c"
    "exosip/eXtl_tcp.c"
    "exosip/eXtl_tls.c"
    "exosip/eXtl_udp.c"
    "exosip/eXtransport.c"
    "exosip/eXutils.c"
    "exosip/inet_ntop.c"
    "exosip/jauth.c"
    "exosip/jcallback.c"
    "exosip/jcall.c"
    "exosip/jdialog.c"
    "exosip/jevents.c"
    "exosip/jnotify.c"
    "exosip/jpipe.c"
    "exosip/jpublish.c"
    "exosip/jreg.c"
    "exosip/jrequest.c"
    "exosip/jresponse.c"
    "exosip/jsubscribe.c"
    "exosip/milenage.c"
    "exosip/misc.c"
    "exosip/rijndael.c"
    "exosip/sdp_offans.c"
    "exosip/udp.c"
)

set(includes . "include")

idf_component_register(SRCS "${srcs}" INCLUDE_DIRS "${includes}" REQUIRES lwip sip)