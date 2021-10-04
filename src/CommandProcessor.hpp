#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <sstream>

namespace comp
{
   using ArgVec = std::vector<std::string>;
   using CommandVec = std::vector<CommandArgs>;
   using ConfigMap = std::unordered_map<std::string, CommandConfig>;

   class Option
   {
   public:

      Option(const std::string& Name = "");

      Option& name(const std::string& Name);
      std::string name() const;

      Option& variadicSize(bool variadicSize);
      bool variadicSize() const;

      Option& argSize(size_t ArgSize);
      size_t argSize() const;

   private:

      std::string m_name;
      bool m_variadicSize;
      size_t m_argSize;
   };

   class CommandConfig
   {
   public:

      using Ptr = std::shared_ptr<CommandConfig>;

      CommandConfig(const std::string& name = "");

      void append(const Option& opt);
      std::string name() const;
      bool has(const std::string& name) const;
      const Option& option(const std::string& name) const;

   private:

      std::string m_name;
      std::unordered_map<std::string, Option> m_options;
   };

   class CommandParser
   {
   public:

      CommandParser() = default;
      CommandParser(const ArgVec& args);

      void init(const ArgVec& args);

      const CommandVec& commands() const;
      const CommandArgs& command(size_t index) const;
      const CommandArgs& command(const std::string& name) const;

      void appendConfig(const CommandConfig::Ptr& pConfig);
      void removeConfig(const std::string& name);
      const CommandConfig::Ptr& config(const std::string& name) const;
      const ConfigMap& configMap() const;

      bool hasCommand(const std::string& name) const;
      bool hasConfig(const std::string& name) const;

      void parse();

   private:

      void parseCommand(const std::string& command, const ArgVec& args);

      ArgVec m_args;
      CommandVec m_commands;
      ConfigMap m_configs;
   };

   class CommandArgs
   {
   public:

      CommandArgs(const ArgVec& args, const CommandConfig& config);
      CommandArgs(const CommandArgs& other);
      CommandArgs(CommandArgs&& other) noexcept;

      CommandArgs& operator=(const CommandArgs& other);
      CommandArgs& operator=(CommandArgs&& other) noexcept;

      ArgVec getStrVec(const std::string& name, bool throwEx = false) const;
      std::string getString(const std::string& name) const;
      std::string getString(const std::string& name, const std::string& defValue) const;
      uint32_t getUInt(const std::string& name) const;
      uint32_t getUInt(const std::string& name, uint32_t defValue) const;
      bool has(const std::string& name) const;

      static std::string tolower(std::string& str);

      std::string command() const;

   private:

      bool isProperty(const std::string& val) const;
      bool isCommand(const std::string& val) const;
      void parse(const ArgVec& args);

      std::unordered_map<std::string, std::string> m_argTable;
      CommandConfig m_config;
   };

   struct CommandStatus
   {
      enum Status : int
      {
         ERROR,
         OK
      };

      CommandStatus(const std::string& name, Status stat = Status::OK, const std::string& msg = "");
      CommandStatus(const CommandStatus& other);
      CommandStatus(CommandStatus&& other) noexcept;

      CommandStatus operator=(const CommandStatus& other);
      CommandStatus operator=(CommandStatus&& other) noexcept;

      std::string name;
      Status status;
      std::string msg;
   };

   class StatusHandler
   {
   public:
      virtual void handle(const CommandStatus& stat) {};
   };

   class CommandCaller
   {
   public:
      template<typename Class>
      using Method = CommandStatus(Class::*)(const CommandArgs& args);
      using Callback = CommandStatus(*)(const CommandArgs& args);

      CommandCaller();

      CommandCaller(Callback callback, const CommandConfig& config);

      template<typename Object>
      CommandCaller(Object* object, Method<Object> method, const CommandConfig& config);

      CommandStatus invoke(const ArgVec& args) const;
      const CommandConfig& config() const;

   private:

      CommandConfig  m_config;
      Callback   m_callback{ nullptr };
      std::function<CommandStatus(const CommandArgs&)> m_invoke{ nullptr };
   };

   class Commander final
   {
   public:

      Commander() = default;
      Commander(const ArgVec& args);

      Commander(const Commander&) = delete;
      Commander& operator=(const Commander&) = delete;

      void init(const ArgVec& args);
      void run();
      void run(const ArgVec& args);

      void appendCommand(const CommandCaller& caller);
      void setHandler(const StatusHandler& handler);

      CommandStatus invokeCommand(const std::string& command, const ArgVec& args);

   private:

      bool isCommand(const std::string& val);

      ArgVec m_args;
      std::unordered_map<std::string, CommandCaller> m_commands;
      StatusHandler m_handler;
   };

   Option::Option(const std::string& Name) :
      m_name(Name),
      m_variadicSize(false),
      m_argSize(0)
   {}

   Option& Option::name(const std::string& Name)
   {
      m_name = Name;
      return *this;
   }

   std::string Option::name() const
   {
      return m_name;
   }

   Option& Option::variadicSize(bool variadicSize)
   {
      m_variadicSize = variadicSize;
      return *this;
   }

   bool Option::variadicSize() const
   {
      return m_variadicSize;
   }

   Option& Option::argSize(size_t ArgSize)
   {
      m_argSize = ArgSize;
      return *this;
   }

   size_t Option::argSize() const
   {
      return m_argSize;
   }

   CommandConfig::CommandConfig(const std::string& name) :
      m_name(name)
   {}

   void CommandConfig::append(const Option& opt)
   {
      m_options[opt.name()] = opt;
   }

   std::string CommandConfig::name() const
   {
      return m_name;
   }

   bool CommandConfig::has(const std::string& name) const
   {
      return (m_options.find(name) != m_options.end());
   }

   const Option& CommandConfig::option(const std::string& name) const
   {
      auto res = m_options.find(name);

      if (res == m_options.end())
         throw std::runtime_error("key \"" + name + "\" not found");

      return res->second;
   }

   CommandParser::CommandParser(const ArgVec& args) :
      m_args(args)
   {}

   void CommandParser::init(const ArgVec& args)
   {
      m_args = args;
   }

   const CommandVec& CommandParser::commands() const
   {
      return m_commands;
   }

   const CommandArgs& CommandParser::command(size_t index) const
   {
      if (index >= m_commands.size())
         throw std::runtime_error("Out of range");

      return m_commands[index];
   }

   const CommandArgs& CommandParser::command(const std::string& name) const
   {
      auto const& it = std::find_if(m_commands.begin(), m_commands.end(),
         [&name](auto const& comm)
         {
            return comm.command() == name;
         });

      if (it == m_commands.end())
         throw std::runtime_error("Out of range");

      return *it;
   }

   void CommandParser::appendConfig(const CommandConfig::Ptr& pConfig)
   {
      m_configs[pConfig->name()] = pConfig;
   }

   void CommandParser::removeConfig(const std::string& name)
   {
      auto it = m_configs.find(name);

      if (it != m_configs.end())
         m_configs.erase(it);
   }

   const CommandConfig::Ptr& CommandParser::config(const std::string& name) const
   {
      auto it = m_configs.find(name);
      return (it != m_configs.end() : it->second : nullptr);
   }

   const ConfigMap& CommandParser::configMap() const
   {
      return m_configs;
   }

   bool CommandParser::hasCommand(const std::string& name) const
   {
      return std::find_if(m_commands.begin(), m_commands.end(),
         [&name](auto const& comm)
         {
            return comm.command() == name;
         }) == m_commands.end();
   }

   bool CommandParser::hasConfig(const std::string& name) const
   {
      return m_configs.find(name) != m_configs.end();
   }

   void CommandParser::parse()
   {
      std::string command;

      //try
      //{
      //   for (size_t i = 0; i < m_args.size(); ++i)
      //   {
      //      if (hasConfig(m_args[i]))
      //      {
      //         command = m_args[i];
      //         ArgVec args = { command };

      //         while (i + 1 < m_args.size() && !hasConfig(m_args[i + 1]))
      //            args.emplace_back(m_args[++i]);

      //         parseCommand(command, args);
      //         //m_handler.handle(invokeCommand(command, args));
      //      }
      //      else
      //      {
      //         m_handler.handle(CommandStatus(m_args[i], CommandStatus::ERROR, "not a command"));
      //         break;
      //      }
      //   }
      //}
      //catch (const std::exception& ex)
      //{
      //   m_handler.handle(CommandStatus(command, CommandStatus::ERROR, ex.what()));
      //}
   }

   void CommandParser::parseCommand(const std::string& command, const ArgVec& args)
   {
      Option unk_opt("unknown");
      unk_opt.argSize(std::numeric_limits<size_t>::max());
      unk_opt.variadicSize(true);

      CommandArgs args;

      for (size_t i = 0; i < args.size();)
      {
         if (args[i] == command && !hasConfig(args[i]))
         {
            ++i;
            continue;
         }

         auto const& command_config = config(command);

         std::string key = command_config->has(args[i]) ? args[i++] : unk_opt.name();
         size_t count = 0;

         auto const& option = (command_config->has(key) ? command_config->option(key) : unk_opt);
         std::string val;

         while (i < args.size() && !command_config->has(args[i]))
         {
            if (count < option.argSize())
            {
               val += args[i++] + ' ';
               ++count;
            }
            else
            {
               break;
            }
         }

         if (!val.empty())
            val.pop_back();

         if (count < option.argSize() && !option.variadicSize())
            throw std::runtime_error("not enough arguments \"" + key + "\"");

         m_argTable[key] = val;
      }
   }

   CommandArgs::CommandArgs(const ArgVec& args, const CommandConfig& config) :
      m_config(config)
   {
      parse(args);
   }

   CommandArgs::CommandArgs(const CommandArgs& other) :
      m_argTable(other.m_argTable),
      m_config(other.m_config)
   {}

   CommandArgs::CommandArgs(CommandArgs&& other) noexcept :
      m_argTable(std::move(other.m_argTable)),
      m_config(std::move(other.m_config))
   {}

   CommandArgs& CommandArgs::operator=(const CommandArgs& other)
   {
      if (this != &other)
      {
         m_argTable = other.m_argTable;
         m_config = other.m_config;
      }

      return *this;
   }

   CommandArgs& CommandArgs::operator=(CommandArgs&& other) noexcept
   {
      if (this != &other)
      {
         m_argTable = std::move(other.m_argTable);
         m_config = std::move(other.m_config);
      }

      return *this;
   }

   ArgVec CommandArgs::getStrVec(const std::string& name, bool throwEx) const
   {
      auto it = m_argTable.find(name);

      if (it == m_argTable.end())
      {
         if (throwEx)
            throw std::runtime_error("key \"" + name + "\" not found");
         else
            return ArgVec{};
      }

      ArgVec res;
      std::stringstream ss(it->second);
      std::string val;

      while (std::getline(ss, val, ' '))
         res.emplace_back(std::move(val));

      return res;
   }

   std::string CommandArgs::getString(const std::string& name) const
   {
      auto res = m_argTable.find(name);

      if (res == m_argTable.end())
         throw std::runtime_error("key \"" + name + "\" not found");

      return res->second;
   }

   std::string CommandArgs::getString(const std::string& name, const std::string& defValue) const
   {
      auto res = m_argTable.find(name);

      if (res == m_argTable.end())
         return defValue;

      return res->second;
   }

   uint32_t CommandArgs::getUInt(const std::string& name) const
   {
      auto res = m_argTable.find(name);

      if (res == m_argTable.end())
         throw std::runtime_error("key \"" + name + "\" not found");

      return std::stoul(res->second);
   }

   uint32_t CommandArgs::getUInt(const std::string& name, uint32_t defValue) const
   {
      auto res = m_argTable.find(name);

      if (res == m_argTable.end())
         return defValue;

      return std::stoul(res->second);
   }

   bool CommandArgs::has(const std::string& name) const
   {
      return (m_argTable.find(name) != m_argTable.end());
   }

   std::string CommandArgs::tolower(std::string& str)
   {
      for (auto& ch : str)
         ch = std::tolower(ch);

      return str;
   }

   std::string CommandArgs::command() const
   {
      return m_config.name();
   }

   bool CommandArgs::isProperty(const std::string& val) const
   {
      return m_config.has(val);
   }

   bool CommandArgs::isCommand(const std::string& val) const
   {
      return val == m_config.name();
   }

   void CommandArgs::parse(const ArgVec& args)
   {
      Option unk_opt("unknown");
      unk_opt.argSize(std::numeric_limits<size_t>::max());
      unk_opt.variadicSize(true);

      for (size_t i = 0; i < args.size();)
      {
         if (isCommand(args[i]) && !m_config.has(args[i]))
         {
            ++i;
            continue;
         }

         std::string key = isProperty(args[i]) ? args[i++] : unk_opt.name();
         size_t count = 0;

         auto const& option = (m_config.has(key) ? m_config.option(key) : unk_opt);
         std::string val;

         while (i < args.size() && !isProperty(args[i]))
         {
            if (count < option.argSize())
            {
               val += args[i++] + ' ';
               ++count;
            }
            else
            {
               break;
            }
         }

         if (!val.empty())
            val.pop_back();

         if (count < option.argSize() && !option.variadicSize())
            throw std::runtime_error("not enough arguments \"" + key + "\"");

         m_argTable[key] = val;
      }
   }

   CommandStatus::CommandStatus(const std::string& Name, Status Stat, const std::string& Msg) :
      name(Name),
      status(Stat),
      msg(Msg)
   {}

   CommandStatus::CommandStatus(const CommandStatus& other) :
      name(other.name),
      status(other.status),
      msg(other.msg)
   {}

   CommandStatus::CommandStatus(CommandStatus&& other) noexcept :
      name(std::move(other.name)),
      status(std::exchange(other.status, CommandStatus::ERROR)),
      msg(std::move(other.msg))
   {}

   CommandStatus CommandStatus::operator=(const CommandStatus& other)
   {
      if (this != &other)
      {
         name = other.name;
         status = other.status;
         msg = other.msg;
      }

      return *this;
   }

   CommandStatus CommandStatus::operator=(CommandStatus&& other) noexcept
   {
      if (this != &other)
      {
         name = std::move(other.name);
         status = std::exchange(other.status, CommandStatus::ERROR);
         msg = std::move(other.msg);
      }

      return *this;
   }

   CommandCaller::CommandCaller()
   {}

   CommandCaller::CommandCaller(Callback callback, const CommandConfig& config) :
      m_config{ config },
      m_callback{ callback }
   {}

   template<typename Object>
   inline CommandCaller::CommandCaller(Object* object, Method<Object> method, const CommandConfig& config) :
      m_config{ config },
      m_invoke{ [object, method](const CommandArgs& args) { return (object->*method)(args); } }
   {}

   CommandStatus CommandCaller::invoke(const ArgVec& args) const
   {
      CommandArgs cargs(args, m_config);
      return (m_callback ? m_callback(cargs) : m_invoke(cargs));
   }

   const CommandConfig& CommandCaller::config() const
   {
      return m_config;
   }

   Commander::Commander(const ArgVec& args) :
      m_args{ args }
   {}

   void Commander::init(const ArgVec& args)
   {
      m_args = args;
   }

   void Commander::run()
   {
      std::string command;

      try
      {
         for (size_t i = 0; i < m_args.size(); ++i)
         {
            if (isCommand(m_args[i]))
            {
               command = m_args[i];
               ArgVec args = { command };

               while (i + 1 < m_args.size() && !isCommand(m_args[i + 1]))
                  args.emplace_back(m_args[++i]);

               m_handler.handle(invokeCommand(command, args));
            }
            else
            {
               m_handler.handle(CommandStatus(m_args[i], CommandStatus::ERROR, "not a command"));
               break;
            }
         }
      }
      catch (const std::exception& ex)
      {
         m_handler.handle(CommandStatus(command, CommandStatus::ERROR, ex.what()));
      }
   }

   inline void Commander::run(const ArgVec& args)
   {
      init(args);
      run();
   }

   void Commander::appendCommand(const CommandCaller& caller)
   {
      m_commands[caller.config().name()] = caller;
   }

   CommandStatus Commander::invokeCommand(const std::string& command, const ArgVec& args)
   {
      return m_commands.at(command).invoke(args);
   }

   void Commander::setHandler(const StatusHandler& handler)
   {
      m_handler = handler;
   }

   bool Commander::isCommand(const std::string& val)
   {
      return (m_commands.find(val) != m_commands.end());
   }
}