#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
db 폴더의 ASP 파일들을 UTF-8(BOM) 인코딩으로 변환하는 스크립트

원본 파일이 EUC-KR/CP949로 인코딩된 경우 한글이 깨지지 않도록 변환합니다.
"""

import os
import codecs

# 변환할 파일 목록
DB_DIR = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'db')

# 시도할 원본 인코딩 목록 (우선순위 순)
SOURCE_ENCODINGS = ['cp949', 'euc-kr', 'utf-8', 'latin-1']

def detect_and_read(filepath):
    """파일을 여러 인코딩으로 시도하여 읽기"""
    for encoding in SOURCE_ENCODINGS:
        try:
            with open(filepath, 'r', encoding=encoding) as f:
                content = f.read()
            # 한글이 깨지지 않았는지 확인 (기본적인 검증)
            if '주문' in content or '결제' in content or '가맹점' in content or '처리' in content:
                print(f"  → 원본 인코딩 감지: {encoding}")
                return content, encoding
        except (UnicodeDecodeError, UnicodeError):
            continue

    # 모든 인코딩 실패시 바이너리로 읽어서 cp949 강제 적용
    print(f"  → 인코딩 감지 실패, cp949 강제 적용")
    with open(filepath, 'rb') as f:
        raw_bytes = f.read()
    return raw_bytes.decode('cp949', errors='replace'), 'cp949 (forced)'

def convert_to_utf8_bom(filepath):
    """파일을 UTF-8 with BOM으로 변환"""
    print(f"\n처리 중: {os.path.basename(filepath)}")

    # 원본 파일 읽기
    content, source_encoding = detect_and_read(filepath)

    # UTF-8 with BOM으로 저장
    with open(filepath, 'w', encoding='utf-8-sig') as f:
        f.write(content)

    print(f"  → UTF-8(BOM)으로 변환 완료")
    return True

def main():
    print("=" * 60)
    print("ASP 파일 인코딩 변환 스크립트")
    print("대상: db 폴더의 모든 .asp 파일")
    print("변환: EUC-KR/CP949 → UTF-8 with BOM")
    print("=" * 60)

    if not os.path.exists(DB_DIR):
        print(f"오류: db 폴더를 찾을 수 없습니다: {DB_DIR}")
        return

    # .asp 파일 목록
    asp_files = [f for f in os.listdir(DB_DIR) if f.endswith('.asp')]

    if not asp_files:
        print("변환할 .asp 파일이 없습니다.")
        return

    print(f"\n발견된 파일 수: {len(asp_files)}개")

    success_count = 0
    for filename in asp_files:
        filepath = os.path.join(DB_DIR, filename)
        try:
            if convert_to_utf8_bom(filepath):
                success_count += 1
        except Exception as e:
            print(f"  → 오류 발생: {e}")

    print("\n" + "=" * 60)
    print(f"변환 완료: {success_count}/{len(asp_files)} 파일")
    print("=" * 60)

if __name__ == "__main__":
    main()
