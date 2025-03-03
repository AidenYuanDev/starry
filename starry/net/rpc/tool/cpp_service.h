#pragma once

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <map>
#include <string>
#include <string_view>

namespace starry {
namespace compiler {

// 声明为extern，这样在多个.cpp文件中包含也不会有多重定义问题
extern const char kThickSeparator[];
extern const char kThinSeparator[];

// 声明为inline函数，这样可以在多个.cpp文件中包含
inline std::string ClassName(const google::protobuf::Descriptor* descriptor,
                    bool qualified) {
  // 在本函数中实现获取类名的逻辑，根据是否需要限定名
  std::string result;
  if (qualified) {
    result = descriptor->file()->package();
    if (!result.empty()) {
      result += "::";
    }
  }

  if (descriptor->containing_type() != nullptr) {
    result += ClassName(descriptor->containing_type(), false) + "_";
  }

  result += descriptor->name();
  return result;
}

inline std::string StripProto(const std::string& filename) {
  if (filename.length() >= 6 &&
      filename.compare(filename.length() - 6, 6, ".proto") == 0) {
    return filename.substr(0, filename.length() - 6);
  }
  return filename;
}

class ServiceGenerator {
 public:
  enum StubOrNon { NON_STUB, STUB };
  enum RequestOrResponse { REQUEST, RESPONSE };
  ServiceGenerator(const google::protobuf::ServiceDescriptor* descriptor,
                   std::string_view filename,
                   int index);
  ~ServiceGenerator();
  ServiceGenerator(const ServiceGenerator&) = delete;
  ServiceGenerator& operator=(const ServiceGenerator&) = delete;

  void GenerateDeclarations(google::protobuf::io::Printer* printer);
  void GenerateImplementation(google::protobuf::io::Printer* printer);

 private:
  void GenerateInterface(google::protobuf::io::Printer* printer);
  void GenerateStubDefinition(google::protobuf::io::Printer* printer);
  void GenerateMethodSignatures(StubOrNon stub_or_non,
                                google::protobuf::io::Printer* printer);
  void GenerateDescriptorInitializer(google::protobuf::io::Printer* printer,
                                     int index);
  void GenerateNotImplementedMethods(google::protobuf::io::Printer* printer);
  void GenerateCallMethod(google::protobuf::io::Printer* printer);
  void GenerateGetPrototype(RequestOrResponse which,
                            google::protobuf::io::Printer* printer);
  void GenerateStubMethods(google::protobuf::io::Printer* printer);

  const google::protobuf::ServiceDescriptor* descriptor_;
  std::map<std::string, std::string> vars_;
};

}  // namespace compiler
}  // namespace starry
