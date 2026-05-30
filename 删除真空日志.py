import os
import re

def remove_vacuum_logs(input_dir, output_dir=None):
    """
    删除以LLA_vacuum、LLB_vacuum、TM_vacuum开头的行
    
    Args:
        input_dir: 输入目录路径
        output_dir: 输出目录路径，默认为输入目录
    """
    # 如果没有指定输出目录，使用输入目录
    if output_dir is None:
        output_dir = input_dir
    
    # 创建输出目录（如果不存在）
    os.makedirs(output_dir, exist_ok=True)
    
    # 要删除的模式列表
    patterns_to_remove = [
        r'^.*LLB_vacuum:',
        r'^.*LLA_vacuum:',
        r'^.*TM_vacuum:',
        r'^.*PM_vacuum:',
        r'^.*pipe_vacuum:'
    ]
    
    # 编译正则表达式模式
    compiled_patterns = [re.compile(pattern) for pattern in patterns_to_remove]
    
    # 遍历目录中的所有txt文件
    for filename in os.listdir(input_dir):
        if filename.endswith('.txt'):
            input_filepath = os.path.join(input_dir, filename)
            
            # 生成输出文件名（添加_cleaned后缀）
            output_filename = os.path.splitext(filename)[0] + '_cleaned.txt'
            output_filepath = os.path.join(output_dir, output_filename)
            
            print(f"处理文件: {filename} -> {output_filename}")
            
            with open(input_filepath, 'r', encoding='utf-8') as infile:
                with open(output_filepath, 'w', encoding='utf-8') as outfile:
                    lines_processed = 0
                    lines_removed = 0
                    lines_kept = 0
                    
                    for line in infile:
                        lines_processed += 1
                        
                        # 检查是否匹配任何要删除的模式
                        should_remove = any(pattern.search(line) for pattern in compiled_patterns)
                        
                        if should_remove:
                            lines_removed += 1
                        else:
                            outfile.write(line)
                            lines_kept += 1
                    
                    print(f"  处理完成: 共{lines_processed}行, 删除{lines_removed}行, 保留{lines_kept}行")
    
    print(f"\n所有文件处理完成！输出目录: {output_dir}")

def remove_vacuum_logs_simple(input_dir, output_dir=None):
    """
    简化版本：使用字符串匹配而不是正则表达式
    """
    if output_dir is None:
        output_dir = input_dir
    
    os.makedirs(output_dir, exist_ok=True)
    
    # 要删除的关键词
    keywords_to_remove = ['LLA_vacuum:', 'LLB_vacuum:', 'TM_vacuum:','PM_vacuum:','pipe_vacuum:']
    
    for filename in os.listdir(input_dir):
        if filename.endswith('.txt'):
            input_filepath = os.path.join(input_dir, filename)
            output_filename = os.path.splitext(filename)[0] + '_cleaned.txt'
            output_filepath = os.path.join(output_dir, output_filename)
            
            print(f"处理文件: {filename} -> {output_filename}")
            
            with open(input_filepath, 'r', encoding='utf-8') as infile:
                lines = infile.readlines()
                
            # 过滤行
            filtered_lines = []
            for line in lines:
                # 检查行中是否包含任何要删除的关键词
                if not any(keyword in line for keyword in keywords_to_remove):
                    filtered_lines.append(line)
            
            # 写入输出文件
            with open(output_filepath, 'w', encoding='utf-8') as outfile:
                outfile.writelines(filtered_lines)
            
            print(f"  处理完成: 原始{len(lines)}行, 过滤后{len(filtered_lines)}行, 删除{len(lines)-len(filtered_lines)}行")
    
    print(f"\n所有文件处理完成！输出目录: {output_dir}")

def batch_process_directory():
    """
    交互式批量处理目录
    """
    print("=== 日志文件清理工具 ===")
    print("功能：删除以LLA_vacuum、LLB_vacuum、TM_vacuum,PM_vacuum,pipe_vacuum开头的行")
    print("-" * 40)
    
    # 获取输入目录
    input_dir = input("请输入包含txt文件的目录路径: ").strip()
    
    # 验证目录是否存在
    if not os.path.isdir(input_dir):
        print(f"错误: 目录 '{input_dir}' 不存在!")
        return
    
    # 列出目录中的txt文件
    txt_files = [f for f in os.listdir(input_dir) if f.endswith('.txt')]
    if not txt_files:
        print(f"错误: 目录 '{input_dir}' 中没有找到txt文件!")
        return
    
    print(f"找到 {len(txt_files)} 个txt文件:")
    for file in txt_files:
        print(f"  - {file}")
    
    # 询问输出目录
    output_dir = input(f"请输入输出目录路径 (回车使用输入目录): ").strip()
    if not output_dir:
        output_dir = input_dir
    
    # 选择处理模式
    print("\n请选择处理模式:")
    print("1. 标准模式（使用正则表达式）")
    print("2. 简单模式（使用字符串匹配）")
    choice = input("请选择 (1 或 2): ").strip()
    
    # 处理文件
    if choice == '1':
        remove_vacuum_logs(input_dir, output_dir)
    elif choice == '2':
        remove_vacuum_logs_simple(input_dir, output_dir)
    else:
        print("无效选择，使用简单模式")
        remove_vacuum_logs_simple(input_dir, output_dir)

def remove_vacuum_from_single_file(input_file, output_file=None):
    """
    处理单个文件
    
    Args:
        input_file: 输入文件路径
        output_file: 输出文件路径，默认为原文件名_cleaned.txt
    """
    if not os.path.isfile(input_file):
        print(f"错误: 文件 '{input_file}' 不存在!")
        return
    
    if output_file is None:
        base, ext = os.path.splitext(input_file)
        output_file = f"{base}_cleaned{ext}"
    
    # 要删除的关键词
    keywords_to_remove = ['LLA_vacuum:', 'LLB_vacuum:', 'TM_vacuum:','PM_vacuum:','pipe_vacuum:']
    
    print(f"处理文件: {os.path.basename(input_file)} -> {os.path.basename(output_file)}")
    
    with open(input_file, 'r', encoding='utf-8') as infile:
        lines = infile.readlines()
    
    # 过滤行
    filtered_lines = []
    for line in lines:
        if not any(keyword in line for keyword in keywords_to_remove):
            filtered_lines.append(line)
    
    # 写入输出文件
    with open(output_file, 'w', encoding='utf-8') as outfile:
        outfile.writelines(filtered_lines)
    
    print(f"处理完成: 原始{len(lines)}行, 过滤后{len(filtered_lines)}行, 删除{len(lines)-len(filtered_lines)}行")
    print(f"输出文件: {output_file}")

def interactive_process_single_file():
    """
    交互式选择单个文件进行处理
    """
    print("=== 单文件真空日志清理工具 ===")
    input_dir = input("请输入包含txt文件的目录路径: ").strip()
    
    # 验证目录是否存在
    if not os.path.isdir(input_dir):
        print(f"错误: 目录 '{input_dir}' 不存在!")
        return
        
    # 获取目录下的所有txt文件
    txt_files = [f for f in os.listdir(input_dir) if f.endswith('.txt')]
    if not txt_files:
        print(f"错误: 目录 '{input_dir}' 中没有找到txt文件!")
        return
        
    print(f"\n找到 {len(txt_files)} 个txt文件:")
    for i, file in enumerate(txt_files, 1):
        print(f"  {i}. {file}")
        
    # 循环等待用户输入正确的序号
    while True:
        choice = input(f"\n请输入要处理的文件序号 (1-{len(txt_files)}，输入 q 退出): ").strip()
        if choice.lower() == 'q':
            return
            
        try:
            idx = int(choice)
            if 1 <= idx <= len(txt_files):
                selected_file = txt_files[idx-1]
                input_filepath = os.path.join(input_dir, selected_file)
                print("-" * 40)
                remove_vacuum_from_single_file(input_filepath)
                break
            else:
                print("错误: 输入的序号超出范围，请重新输入。")
        except ValueError:
            print("错误: 无效的输入，请输入数字序号。")

if __name__ == "__main__":
    # 交互式选择目录及文件进行处理
    interactive_process_single_file()