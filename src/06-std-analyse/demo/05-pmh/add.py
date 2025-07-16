def generate_kv_pairs(n: int) -> None:
    print("{")
    for i in range(n):
        key = f"key{i:03}"
        print(f'    {{"{key}", {i}}},')
    print("}")

# 示例：生成 10 个
generate_kv_pairs(1024)
