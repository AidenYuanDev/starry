#pragma once

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <map>
#include <string>
#include <string_view>

namespace starry {
namespace compiler {

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

std::string ClassName(const google::protobuf::Descriptor* descriptor,
                      bool qualified);
std::string StripProto(const std::string& filename);

extern const char kThickSeparator[];
extern const char kThinSeparator[];

}  // namespace compiler
}  // namespace starry
