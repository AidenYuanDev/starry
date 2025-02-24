#include <memory>
#include <string>
#include <vector>

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/stubs/common.h>

#include "cpp_service.h"

namespace starry {
namespace compiler {

class RpcGenerate : public google::protobuf::compiler::CodeGenerator {
 public:
  RpcGenerate() = default;
  ~RpcGenerate() override = default;

  bool Generate(const google::protobuf::FileDescriptor* file,
                const std::string&,
                google::protobuf::compiler::GeneratorContext* generator_context,
                std::string*) const override {
    if (file->service_count() == 0) {
      return true;
    }

    std::string basename = StripProto(file->name()) + ".pb";

    // 引入头文件
    {
      std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> output(
          generator_context->OpenForInsert(basename + ".h", "includes"));
      google::protobuf::io::Printer::Options options;
      options.variable_delimiter = '$';
      google::protobuf::io::Printer printer(output.get(), options);
      printer.Print("#include <starry/rpc/service.h>\n");
      printer.Print("#include <memory>\n");
      printer.Print("#include <functional>\n");
    }

    // 为每个服务创建生成器
    std::vector<std::unique_ptr<ServiceGenerator>> service_generators;
    for (int i = 0; i < file->service_count(); i++) {
      service_generators.emplace_back(
          new ServiceGenerator(file->service(i), file->name(), i));
    }

    // 生成头文件
    {
      std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> output(
          generator_context->OpenForInsert(basename + ".h", "namespace_scope"));
      google::protobuf::io::Printer::Options options;
      options.variable_delimiter = '$';
      google::protobuf::io::Printer printer(output.get(), options);

      printer.Print("\n");
      printer.Print(kThickSeparator);
      printer.Print("\n");

      // 生成消息的前向声明
      for (int i = 0; i < file->message_type_count(); i++) {
        printer.Print("using $classname$Ptr = std::shared_ptr<$classname$>;\n",
                      "classname", ClassName(file->message_type(i), false));
      }

      // 生成服务定义
      for (int i = 0; i < file->service_count(); i++) {
        printer.Print("\n");
        printer.Print(kThinSeparator);
        printer.Print("\n");
        service_generators[i]->GenerateDeclarations(&printer);
      }
    }
    // 生成源文件内容
    {
      std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> output(
          generator_context->OpenForInsert(basename + ".cc",
                                           "namespace_scope"));
      google::protobuf::io::Printer::Options options;
      options.variable_delimiter = '$';
      google::protobuf::io::Printer printer(output.get(), options);

      // 生成服务实现
      for (int i = 0; i < file->service_count(); i++) {
        printer.Print(kThinSeparator);
        printer.Print("\n");
        service_generators[i]->GenerateImplementation(&printer);
        printer.Print("\n");
      }
    }
    return true;
  }
};

const char kThickSeparator[] =
    "// ===================================================================\n";
const char kThinSeparator[] =
    "// -------------------------------------------------------------------\n";

std::string ClassName(const google::protobuf::Descriptor* descriptor, bool) {
  return descriptor->name();
}

std::string StripProto(const std::string& filename) {
  if (filename.length() >= 6 &&
      filename.compare(filename.length() - 6, 6, ".proto") == 0) {
    return filename.substr(0, filename.length() - 6);
  }
  return filename;
}

}  // namespace compiler
}  // namespace starry

int main(int argc, char* argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  starry::compiler::RpcGenerate generator;
  return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
