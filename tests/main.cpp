#define DOCTEST_CONFIG_IMPLEMENT

#include "doctest.h"

// doctest comments
// 'function' : must be 'attribute' - see issue #182
// DOCTEST_MSVC_SUPPRESS_WARNING_WITH_PUSH(4007)
// int main(int argc, char** argv) { return doctest::Context(argc, argv).run(); }
// DOCTEST_MSVC_SUPPRESS_WARNING_POP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

int main() {
 MYSQL *conn;
 MYSQL_STMT *stmt;
 MYSQL_BIND result_bind[2]; // 假设我们要获取id和flag_bit两列
 MYSQL_RES *prepare_meta_result;

 // 步骤1: 初始化MySQL连接对象
 conn = mysql_init(NULL);
 if (conn == NULL) {
   fprintf(stderr, "mysql_init() failed\n");
   return 1;
 }

 // 步骤2: 建立数据库连接
 if (mysql_real_connect(conn, "localhost", "root", "123456",
                        "test_ormppdb", 0, NULL, 0) == NULL) {
   fprintf(stderr, "mysql_real_connect() failed\n");
   fprintf(stderr, "%s\n", mysql_error(conn));
   mysql_close(conn);
   return 1;
 }

 // 步骤3: 创建预处理语句对象
 stmt = mysql_stmt_init(conn);
 if (stmt == NULL) {
   fprintf(stderr, "mysql_stmt_init() failed\n");
   mysql_close(conn);
   return 1;
 }

 // 步骤4: 准备SQL语句 - 查询所有行
 const char *query = "SELECT id, bit_field FROM bit_test";
 if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
   fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(stmt));
   mysql_stmt_close(stmt);
   mysql_close(conn);
   return 1;
 }

 // 步骤5: 获取结果集元数据
 prepare_meta_result = mysql_stmt_result_metadata(stmt);
 if (prepare_meta_result == NULL) {
   fprintf(stderr, "mysql_stmt_result_metadata() failed: %s\n", mysql_stmt_error(stmt));
   mysql_stmt_close(stmt);
   mysql_close(conn);
   return 1;
 }

 // 获取列数
 unsigned int column_count = mysql_num_fields(prepare_meta_result);
 printf("Number of columns: %d\n", column_count);

 // 为结果数据准备变量
 int id_value;
 int bit_value;
 unsigned long id_length, bit_length;
 bool id_is_null, bit_is_null;

 // 步骤6: 绑定结果集
 memset(result_bind, 0, sizeof(result_bind));

 // 绑定id列
 result_bind[0].buffer_type = MYSQL_TYPE_LONG;
 result_bind[0].buffer = (void *)&id_value;
 result_bind[0].buffer_length = sizeof(id_value);
 result_bind[0].is_null = &id_is_null;
 result_bind[0].length = &id_length;

 // 绑定flag_bit列
 result_bind[1].buffer_type = MYSQL_TYPE_LONG;
 result_bind[1].buffer = (void *)&bit_value;
 result_bind[1].buffer_length = sizeof(bit_value);
 result_bind[1].is_null = &bit_is_null;
 result_bind[1].length = &bit_length;

 if (mysql_stmt_bind_result(stmt, result_bind) != 0) {
   fprintf(stderr, "mysql_stmt_bind_result() failed: %s\n", mysql_stmt_error(stmt));
   mysql_free_result(prepare_meta_result);
   mysql_stmt_close(stmt);
   mysql_close(conn);
   return 1;
 }

 // 步骤7: 执行预处理语句
 if (mysql_stmt_execute(stmt) != 0) {
   fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
   mysql_free_result(prepare_meta_result);
   mysql_stmt_close(stmt);
   mysql_close(conn);
   return 1;
 }

 // 步骤8: 将结果集存储到客户端
 if (mysql_stmt_store_result(stmt) != 0) {
   fprintf(stderr, "mysql_stmt_store_result() failed: %s\n", mysql_stmt_error(stmt));
   mysql_free_result(prepare_meta_result);
   mysql_stmt_close(stmt);
   mysql_close(conn);
   return 1;
 }

 // 获取行数
 unsigned long long row_count = mysql_stmt_num_rows(stmt);
 printf("Number of rows: %llu\n", row_count);

 // 步骤9: 循环获取所有结果行
 printf("\nQuery Results:\n");
 printf("ID\tBIT Value\n");
 printf("------------------\n");

 int row = 0;
 int status;

 // 使用循环遍历所有结果行
 while ((status = mysql_stmt_fetch(stmt)) == 0) {
   row++;

   // 处理id列
   if (id_is_null) {
     printf("NULL\t");
   } else {
     printf("id: %d\t", id_value);
   }

   // 处理flag_bit列
   if (bit_is_null) {
     printf("NULL\n");
   } else {
     // 对于BIT(1)，通常只关注最低位
     printf("bit: %d\n", bit_value);

     // 如果需要查看原始字节值，可以取消下面注释
     // printf("(raw: %u)\n", bit_value);
   }
 }

 // 检查循环是否因为错误而终止
 if (status != MYSQL_NO_DATA) {
   fprintf(stderr, "mysql_stmt_fetch() failed: %s\n", mysql_stmt_error(stmt));
 }

 printf("------------------\n");
 printf("Total rows: %d\n", row);

 // 步骤10: 清理资源
 mysql_free_result(prepare_meta_result);
 mysql_stmt_close(stmt);
 mysql_close(conn);

 return 0;
}