// common1.cpp

// ldap attribute for devices

#define DEVICE_ATTR_IPHOST_POS      0
#define DEVICE_ATTR_IPHOST_STRING   "tnetLrpIpHostNumber"
#define DEVICE_ATTR_KEYPUB_POS      1
#define DEVICE_ATTR_KEYPUB_STRING   "tnetLrpAdminKeyPub"
#define DEVICE_ATTR_KEYPRIV_POS     2
#define DEVICE_ATTR_KEYPRIV_STRING  "tnetLrpAdminKeyPriv"
#define DEVICE_NUM_ATTR             3

static const char* device_attr_name[] = { DEVICE_ATTR_IPHOST_STRING, DEVICE_ATTR_KEYPUB_STRING,
                                          DEVICE_ATTR_KEYPRIV_STRING, 0 };


   // manage arg operation

   const char* operation = argv[optind++];

   if (operation == 0) U_ERROR("<operation(apply|drop)> not specified");

   // manage file configuration

   int LDAP_port = 0;
   UString LDAP_host, LDAP_binddn, LDAP_password, username, DN_filter, value, directory, LDAP_binddn_device;

   value = (UApplication::isOptions() ? opt['c'] : U_STRING_FROM_CONSTANT("lrp.cfg"));

   UFileConfig cfg;

   cfg.open(value);

   UString host_key      = U_STRING_FROM_CONSTANT("HOST"),
           port_key      = U_STRING_FROM_CONSTANT("PORT"),
           binddn_key    = U_STRING_FROM_CONSTANT("BIND_DN"),
           binddndev_key = U_STRING_FROM_CONSTANT("BIND_DN_DEVICE"),
           password_key  = U_STRING_FROM_CONSTANT("BIND_PASSWORD"),
           username_key  = U_STRING_FROM_CONSTANT("SSH_USERNAME"),
           filter_key    = U_STRING_FROM_CONSTANT("DN_FILTER"),
           dirlog_key    = U_STRING_FROM_CONSTANT("DIRECTORY_FOR_LOG");

   LDAP_host          = cfg[host_key];
   LDAP_binddn        = cfg[binddn_key];
   LDAP_binddn_device = cfg[binddndev_key];
   LDAP_password      = cfg[password_key];
   username           = cfg[username_key];
   DN_filter          = cfg[filter_key];
   directory          = cfg[dirlog_key];
   LDAP_port          = cfg.readLong(port_key, 389);

   // manage options

   if (UApplication::isOptions())
      {
      int port = opt['p'].strtol();
      if (port) LDAP_port = port;

      value = opt['H'];
      if (value.empty() == false) LDAP_host = value;

      value = opt['D'];
      if (value.empty() == false) LDAP_binddn = value;

      value = opt['w'];
      if (value.empty() == false) LDAP_password = value;

      value = opt['u'];
      if (value.empty() == false) username = value;

      value = opt['B'];
      if (value.empty() == false) LDAP_binddn_device = value;
      }

   // login to LDAP

   ULDAP ldap;

   if (ldap.init(LDAP_host.c_str(), (LDAP_port ? LDAP_port : LDAP_PORT)) == false ||
       ldap.set_protocol() == false ||
       ldap.bind(LDAP_binddn.c_str(), LDAP_password.c_str()) == false)
      {
      U_ERROR("login to LDAP failed");
      }

   // check for directory value

   if (directory.empty()) directory = U_STRING_FROM_CONSTANT(".");

   // check for bind dn device value

   if (LDAP_binddn_device.empty()) LDAP_binddn_device = U_STRING_FROM_CONSTANT("o=Devices,o=tnet");
