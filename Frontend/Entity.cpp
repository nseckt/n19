/*
* Copyright (c) 2024 Diago Lima
* SPDX-License-Identifier: BSD-3-Clause
*/

#include <Frontend/Entity.hpp>
#include <IO/Console.hpp>
#include <Frontend/EntityTable.hpp>
#include <IO/Fmt.hpp>
BEGIN_NAMESPACE(n19);

BuiltinType::BuiltinType(const Type type) {
  #define X(TYPE, STR, _1) if(type == TYPE){ lname_ = STR; name_ = "::" STR; }
  N19_ENTITY_BUILTIN_LIST
  #undef X
}

auto EntityQualifier::format() const -> std::string {
  std::string buff;

  #define X(EQ_FLAG, UNUSED)                      \
  if(flags_ & n19::EntityQualifierBase::EQ_FLAG){ \
    buff += #EQ_FLAG " | ";                       \
  }
    N19_EQ_FLAG_LIST
  #undef X

  if(!buff.empty()) {
    buff.erase(buff.size() - 3);
    buff += ", ";
  }

  buff += fmt("ptr_depth = {}, ", ptr_depth_);
  buff += arr_lengths_.empty() ? "array_lengths = N/A " : "array_lengths = ";

  for(const auto& length : arr_lengths_) {
    buff += fmt("{} ", length);
  }

  return fmt(
    "{}ID: {} {}{}{}{}",         /// Fmt string
    manip_string(Con::BlueFG),   /// ID color = blue
    static_cast<uint32_t>(id_),  /// The ID
    manip_string(Con::Reset),    /// Reset console
    manip_string(Con::WhiteFG),  /// buff = white
    buff,                        /// The buff
    manip_string(Con::Reset));   /// Reset console
}

auto EntityQualifierThunk::format() const -> std::string {
  std::string buff;      /// the flags and qualifiers
  std::string full_name; /// full entity name

  #define X(EQ_FLAG, UNUSED)                      \
  if(flags_ & n19::EntityQualifierBase::EQ_FLAG){ \
    buff += #EQ_FLAG " | ";                       \
  }
    N19_EQ_FLAG_LIST
  #undef X

  if(!buff.empty()) {
    buff.erase(buff.size() - 3);
    buff += ", ";
  }

  buff += fmt("ptr_depth = {}, ", ptr_depth_);
  buff += arr_lengths_.empty() ? "array_lengths = N/A " : "array_lengths = ";

  for(const auto& length : arr_lengths_) {
    buff.append(fmt("{} ", length));
  } for(const auto& chunk : name_) {
    full_name.append(chunk);
  }

  return fmt(
    "{}{} {}{}{}{}",             /// Fmt string
    manip_string(Con::BlueFG),   /// ID color = blue
    full_name,                   /// The ID
    manip_string(Con::Reset),    /// Reset console
    manip_string(Con::WhiteFG),  /// buff = white
    buff,                        /// The buff
    manip_string(Con::Reset));   /// Reset console
}

auto EntityQualifier::get_const_bool() -> EntityQualifier {
  EntityQualifier const_bool;
  const_bool.id_    = BuiltinType::Bool;
  const_bool.flags_ = Constant;
  return const_bool;
}

auto EntityQualifier::get_const_f64() -> EntityQualifier {
  EntityQualifier const_f64;
  const_f64.id_     = BuiltinType::F64;
  const_f64.flags_  = Constant;
  return const_f64;
}

auto EntityQualifier::get_const_ptr() -> EntityQualifier {
  EntityQualifier const_ptr;
  const_ptr.id_        = BuiltinType::Ptr;
  const_ptr.ptr_depth_ = 1;
  const_ptr.flags_     = Constant;
  return const_ptr;
}

auto EntityQualifier::to_string(
  const EntityTable &tbl,
  const bool include_qualifiers,
  const bool include_postfixes ) const -> std::string
{
  std::string buff;
  const auto ent = tbl.find(id_);

  if(include_qualifiers) {
    #define X(VAL, UNUSED) if(flags_ & VAL) buff.append(#VAL " ");
    N19_EQ_FLAG_LIST /* convert to string repr */
    #undef X
  }

  buff += ent->name_;
  if(include_postfixes) {
    for(const auto& len : arr_lengths_) buff.append(fmt("[{}]", len));
    for(int i = 0; i < ptr_depth_; i++) buff.append("*");
  }

  return buff;
}

auto EntityQualifierThunk::to_string(
  const bool include_qualifiers,
  const bool include_postfixes ) const -> std::string
{
  std::string buff;
  if(include_qualifiers) {
    #define X(VAL, UNUSED) if(flags_ & VAL) buff.append(#VAL " ");
    N19_EQ_FLAG_LIST /* convert to string repr */
    #undef X
  }

  for(const auto& chunk : name_) {
    buff.append(chunk);
  }

  if(include_postfixes) {
    for(int i = 0; i < ptr_depth_; i++) buff.append("*");
    for(const auto& len : arr_lengths_) buff.append(fmt("[{}]", len));
  }

  return buff;
}

auto Entity::print_children_(
  const uint32_t depth,
  OStream &stream, EntityTable &table ) const -> void
{
  stream << "\n";
  for(Entity::ID id : chldrn_) {
    auto ptr = table.find(id);
    ptr->print(depth + 1, stream, table);
  }
}

auto Entity::print_(
  const uint32_t depth,
  OStream &stream ) const -> void
{
  for(uint32_t i = 0; i < depth; i++)
    stream << "  |";
  if(depth)
    stream << "_ ";

  /// Preamble
  stream
    << Con::Bold
    << Con::MagentaFG
    << this->name_
    << Con::Reset;
  stream
    << " <"
    << Con::YellowFG
    << this->line_
    << Con::Reset
    << ','
    << Con::YellowFG
    << this->pos_
    << Con::Reset
    << "> -- ";

  /// Display entity type
#define X(TYPE)              \
  case EntityType::TYPE:     \
  stream                     \
    << Con::GreenFG          \
    << #TYPE                 \
    << Con::Reset            \
    << " with ";             \
  break;

  switch(type_) {
    N19_ENTITY_TYPE_LIST
    default: UNREACHABLE_ASSERTION;
  }
#undef X

  /// Entity ID and other info
  stream
    << "ID="
    << Con::GreenFG
    << this->id_
    << Con::Reset
    << ", File="
    << Con::GreenFG
    << "\""
    << this->file_
    << "\""
    << Con::Reset
    << " ";
}

auto Type::print(
  const uint32_t depth,
  OStream &stream, EntityTable &table ) const -> void
{
  print_(depth, stream);
  print_children_(depth, stream, table);
}

auto Struct::print(
  const uint32_t depth,
  OStream &stream, EntityTable &table ) const -> void
{
  print_(depth, stream);
  print_children_(depth, stream, table);
}

auto BuiltinType::print(
  const uint32_t depth,
  OStream &stream, EntityTable &table) const -> void
{
  print_(depth, stream);
  print_children_(depth, stream, table);
}

auto AliasType::print(
  const uint32_t depth,
  OStream &stream, EntityTable &table ) const -> void
{
  print_(depth, stream);
  stream
    << ", Link="
    << Con::BlueFG
    << this->link_
    << Con::Reset
    << Con::Bold
    << ", Qualifiers: "
    << Con::Reset;

  std::string buff;

  #define X(EQ_FLAG, UNUSED)                             \
  if(quals_.flags_ & n19::EntityQualifierBase::EQ_FLAG){ \
    buff += #EQ_FLAG " | ";                              \
  }
    N19_EQ_FLAG_LIST
  #undef X

  if(!buff.empty()) {
    buff.erase(buff.size() - 3);
    buff += ", ";
  }

  buff += fmt("ptr_depth = {}, ", quals_.ptr_depth_);
  buff += quals_.arr_lengths_.empty() ? "array_lengths = N/A " : "array_lengths = ";

  for(const auto& length : quals_.arr_lengths_) {
    buff += fmt("{} ", length);
  }

  stream << "\n";
  print_children_(depth, stream, table);
}

auto Static::print(
  const uint32_t depth,
  OStream& stream, EntityTable &table ) const -> void
{
  print_(depth, stream);
  print_children_(depth, stream, table);
}

auto Proc::print(
  const uint32_t depth,
  OStream &stream, EntityTable &table) const -> void
{
  print_(depth, stream);
  stream << "Parameters: { " << Con::BlueFG;
  for(Entity::ID id : parameters_) {
    stream << id << " ";
  }

  stream
    << Con::Reset
    << "}, "
    << "ReturnType="
    << Con::BlueFG
    << return_type_
    << Con::Reset;
  print_children_(depth, stream, table);
}

auto PlaceHolder::print(
  const uint32_t depth,
  OStream &stream, EntityTable &table ) const -> void
{
  print_(depth, stream);
  stream
    << Con::RedFG
    << "(PLACEHOLDER)"
    << Con::Reset;
  print_children_(depth, stream, table);
}

auto SymLink::print(
  const uint32_t depth,
  OStream &stream, EntityTable &table ) const -> void
{
  print_(depth, stream);
  stream
    << ", Link="
    << Con::BlueFG
    << this->link_
    << Con::Reset;
  print_children_(depth, stream, table);
}

auto Variable::print(
  const uint32_t depth,
  OStream &stream, EntityTable &table ) const -> void
{
  print_(depth, stream);
  print_children_(depth, stream, table);
}

auto RootEntity::print(
  const uint32_t depth,
  OStream &stream, EntityTable &table ) const -> void
{
  print_(depth, stream);
  stream
    << Con::RedFG
    << "(ROOT)"
    << Con::Reset;
  print_children_(depth, stream, table);
}

END_NAMESPACE(n19);