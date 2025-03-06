#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <cassert>
#include <cstddef>
#include <string>

#include "cpp_plugin.h"

bool CppStarryGenerator::Generate(
    const google::protobuf::FileDescriptor* file,
    const std::string& parameter,
    google::protobuf::compiler::GeneratorContext* context,
    std::string* error) const {
  if (file->package().empty()) {
    error->append("package name is missed.");
    return false;
  }

  std::string cpp_name = protoBaseName(file->name()) + ".pb.cc";

  google::protobuf::io::Printer inc_printer(
      context->OpenForInsert(cpp_name, "includes"), '$');
  genHeader(inc_printer);

  google::protobuf::io::Printer gdecl_printer(
      context->OpenForInsert(cpp_name, "includes"), '$');
  google::protobuf::io::Printer gimpl_printer(
      context->OpenForInsert(cpp_name, "global_scope"), '$');
}

std::string CppStarryGenerator::protoBaseName(
    const std::string& filename) const {
  std::size_t pos = filename.find_last_of(".");
  assert(pos != std::string::npos);

  return filename.substr(0, pos);
}

void CppStarryGenerator::genHeader(google::protobuf::io::Printer& printer) {
  printer.Print("#include \"service.h\"\n");
  printer.Print("#include <memory.h>\n");
}
