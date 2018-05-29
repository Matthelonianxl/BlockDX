//******************************************************************************
//******************************************************************************
#ifndef XROUTERSETTINGS_H
#define XROUTERSETTINGS_H

#include <vector>
#include <string>

#include <boost/property_tree/ptree.hpp>

#define TRY(_STMNT_) try { (_STMNT_); } catch(std::exception & e) { LOG() << e.what(); }

//******************************************************************************
class XRouterSettings
{
public:
    XRouterSettings();

    bool parseCmdLine(int, char * argv[]);

    bool read(const char * fileName = 0);

public:
    bool isFullLog()
        { return get<bool>("Main.FullLog", false); }

    bool isExchangeEnabled() const { return m_isExchangeEnabled; }
    std::string appPath() const    { return m_appPath; }

    std::string logPath() const;

    bool rpcEnabled()
        { return get<bool>("Rpc.Enable", false); }
    uint32_t rpcPort(const uint32_t def)
        { return get<uint32_t>("Rpc.Port", def); }

    std::string rpcServerUserName()
        { return get<std::string>("Rpc.UserName"); }
    std::string rpcServerPasswd()
        { return get<std::string>("Rpc.Password"); }
    bool rpcUseSsl()
        { return get<bool>("Rpc.UseSSL"); }
    std::string rpcSertFile(const std::string def = std::string())
        { return get<std::string>("Rpc.SertFile", def); }
    std::string rpcPKeyFile(const std::string def = std::string())
        { return get<std::string>("Rpc.PKeyFile", def); }
    std::string rpcSslCiphers(const std::string def = std::string())
        { return get<std::string>("Rpc.SslCiphers", def); }

public:
    template <class _T>
    _T get(const std::string & param, _T def = _T())
    {
        return get<_T>(param.c_str(), def);
    }

    template <class _T>
    _T get(const char * param, _T def = _T())
    {
        _T tmp = def;
        try
        {
            tmp = m_pt.get<_T>(param);
            return tmp;
        }
        catch (std::exception & e)
        {
            //LOG() << e.what();
        }

        return tmp;
    }

private:
    std::string                 m_appPath;
    std::string                 m_fileName;
    boost::property_tree::ptree m_pt;

    std::vector<std::string>    m_peers;

    bool                        m_isExchangeEnabled;
};

#endif // SETTINGS_H
