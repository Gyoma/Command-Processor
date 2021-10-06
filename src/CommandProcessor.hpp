#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <sstream>
#include <memory>

namespace comp
{
   using ArgVec = std::vector<std::string>;
   using CommandVec = std::vector<class CommandArgs>;
   using ConfigMap = std::unordered_map<std::string, std::shared_ptr<class CommandConfig>>;

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

      void appendConfig(CommandConfig::Ptr pConfig);
      void removeConfig(const std::string& name);
      CommandConfig::Ptr config(const std::string& name) const;
      const ConfigMap& configMap() const;

      bool hasCommand(const std::string& name) const;
      bool hasConfig(const std::string& name) const;

      void parse();
      void parse(const ArgVec& args);

   private:

      CommandArgs parseCommand(const std::string& command, const ArgVec& args);

      ArgVec m_args;
      CommandVec m_commands;
      ConfigMap m_configs;
   };

   class CommandArgs
   {
   public:

      CommandArgs() = default;
      explicit CommandArgs(const std::string& command);

      CommandArgs(const CommandArgs& other);
      CommandArgs(CommandArgs&& other) noexcept;

      CommandArgs& operator=(const CommandArgs& other);
      CommandArgs& operator=(CommandArgs&& other) noexcept;

      void insert(const std::string& key, const std::string& val);
      void remove(const std::string& key);

      ArgVec getStrVec(const std::string& name, bool throwEx = false) const;
      std::string getString(const std::string& name) const;
      std::string getString(const std::string& name, const std::string& defValue) const;
      uint32_t getUInt(const std::string& name) const;
      uint32_t getUInt(const std::string& name, uint32_t defValue) const;
      bool has(const std::string& name) const;

      static std::string tolower(std::string& str);

      void setCommand(const std::string& command);
      std::string command() const;

   private:

      std::string m_command;
      std::unordered_map<std::string, std::string> m_argTable;
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
      virtual int32_t handle(const CommandStatus& stat) { return 0; };
   };

   class CommandCaller
   {
   public:
      template<typename Class>
      using Method = CommandStatus(Class::*)(const CommandArgs& args);
      using Callback = CommandStatus(*)(const CommandArgs& args);

      CommandCaller();

      CommandCaller(Callback callback, CommandConfig::Ptr config);

      template<typename Object>
      CommandCaller(Object* object, Method<Object> method, CommandConfig::Ptr config);

      CommandStatus invoke(const ArgVec& args) const;
      CommandStatus invoke(const CommandArgs& args) const;

      CommandConfig::Ptr config() const;

   private:

      CommandConfig::Ptr  m_config;
      std::function<CommandStatus(const CommandArgs&)> m_callback{ nullptr };
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
      void removeCommand(const std::string& callerName);

      void setHandler(const StatusHandler& handler);

      CommandStatus invokeCommand(const std::string& command, const ArgVec& args) const;
      CommandStatus invokeCommand(const CommandArgs& args) const;

   private:

      bool isCommand(const std::string& val);

      ArgVec m_args;
      CommandParser m_parser;
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

   void CommandParser::appendConfig(CommandConfig::Ptr pConfig)
   {
      m_configs[pConfig->name()] = std::move(pConfig);
   }

   void CommandParser::removeConfig(const std::string& name)
   {
      auto it = m_configs.find(name);

      if (it != m_configs.end())
         m_configs.erase(it);
   }

   CommandConfig::Ptr CommandParser::config(const std::string& name) const
   {
      auto it = m_configs.find(name);
      return (it != m_configs.end() ? it->second : nullptr);
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
      for (size_t i = 0; i < m_args.size();)
      {
         std::string command = hasConfig(m_args[i]) ? m_args[i++] : "unknown";
         ArgVec args = { command };

         while (i < m_args.size() && !hasConfig(m_args[i]))
            args.push_back(m_args[i++]);

         m_commands.push_back(parseCommand(command, args));
      }
   }

   void CommandParser::parse(const ArgVec& args)
   {
      init(args);
      parse();
   }

   CommandArgs CommandParser::parseCommand(const std::string& command, const ArgVec& args)
   {
      CommandArgs comargs(command);

      Option unk_opt("unknown");
      unk_opt.argSize(std::numeric_limits<size_t>::max());
      unk_opt.variadicSize(true);

      auto conf = config(command);

      for (size_t i = 0; i < args.size();)
      {
         if (args[i] == command && !(conf && conf->has(command)))
         {
            ++i;
            continue;
         }

         std::string val;
         size_t count = 0;
         auto const& option = (conf && conf->has(args[i]) ? conf->option(args[i++]) : unk_opt);

         while (i < args.size() && !(conf && conf->has(args[i])))
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
            throw std::runtime_error("not enough arguments \"" + option.name() + "\"");

         comargs.insert(option.name(), val);
      }

      return comargs;
   }

   CommandArgs::CommandArgs(const std::string& command) :
      m_command(command)
   {}

   CommandArgs::CommandArgs(const CommandArgs& other) :
      m_command(other.m_command),
      m_argTable(other.m_argTable)
   {}

   CommandArgs::CommandArgs(CommandArgs&& other) noexcept :
      m_command(std::move(other.m_command)),
      m_argTable(std::move(other.m_argTable))
   {}

   CommandArgs& CommandArgs::operator=(const CommandArgs& other)
   {
      if (this != &other)
      {
         m_command = other.m_command;
         m_argTable = other.m_argTable;
      }

      return *this;
   }

   CommandArgs& CommandArgs::operator=(CommandArgs&& other) noexcept
   {
      if (this != &other)
      {
         m_command = std::move(other.m_command);
         m_argTable = std::move(other.m_argTable);
      }

      return *this;
   }

   void CommandArgs::insert(const std::string& key, const std::string& val)
   {
      m_argTable[key] = val;
   }

   void CommandArgs::remove(const std::string& key)
   {
      m_argTable.erase(key);
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

   void CommandArgs::setCommand(const std::string& command)
   {
      m_command = command;
   }

   std::string CommandArgs::command() const
   {
      return m_command;
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

   CommandCaller::CommandCaller(Callback callback, CommandConfig::Ptr config) :
      m_config{ std::move(config) },
      m_callback{ callback }
   {}

   template<typename Object>
   inline CommandCaller::CommandCaller(Object* object, Method<Object> method, CommandConfig::Ptr config) :
      m_config{ std::move(config) },
      m_callback{ [object, method](const CommandArgs& args) { return (object->*method)(args); } }
   {}

   CommandStatus CommandCaller::invoke(const ArgVec& args) const
   {
      CommandParser cparser;
      cparser.appendConfig(m_config);
      cparser.parse(args);
      CommandArgs cargs = cparser.command(0);

      return invoke(cargs);
   }

   CommandStatus CommandCaller::invoke(const CommandArgs& args) const
   {
      return m_callback(args);
   }

   CommandConfig::Ptr CommandCaller::config() const
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
      m_parser.parse(m_args);
      int32_t state = 0;

      const CommandVec& commands = m_parser.commands();

      for (size_t i = 0; state == 0 && i < commands.size(); ++i)
      {
         const CommandArgs& commandArgs = commands[i];

         try
         {
            if (isCommand(commandArgs.command()))
            {
               state = m_handler.handle(invokeCommand(commandArgs));
            }
            else
            {
               state = m_handler.handle(CommandStatus(commandArgs.command(), CommandStatus::ERROR, "not a command"));
            }
         }
         catch (const std::exception& ex)
         {
            state = m_handler.handle(CommandStatus(commandArgs.command(), CommandStatus::ERROR, ex.what()));
         }
      }
   }

   inline void Commander::run(const ArgVec& args)
   {
      init(args);
      run();
   }

   void Commander::appendCommand(const CommandCaller& caller)
   {
      m_commands[caller.config()->name()] = caller;
      m_parser.appendConfig(caller.config());
   }


   void Commander::removeCommand(const std::string& callerName)
   {
      m_commands.erase(callerName);
      m_parser.removeConfig(callerName);
   }

   CommandStatus Commander::invokeCommand(const std::string& command, const ArgVec& args) const
   {
      return m_commands.at(command).invoke(args);
   }

   CommandStatus Commander::invokeCommand(const CommandArgs& commandArgs) const
   {
      return m_commands.at(commandArgs.command()).invoke(commandArgs);
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