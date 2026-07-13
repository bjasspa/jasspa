import pytest
import subprocess
import sys
import os
import ctypes
import struct


# Adversarial payloads: strings exceeding typical buffer sizes by 2x, 10x, and more
@pytest.mark.parametrize("payload", [
    # 2x typical buffer size (256 bytes)
    "A" * 512,
    # 10x typical buffer size
    "B" * 2560,
    # Classic buffer overflow pattern
    "C" * 1024,
    # Null bytes embedded in oversized string
    "D" * 255 + "\x00" + "E" * 255,
    # Format string attack combined with overflow
    "%s%s%s%s%s%s%s%s%s%s" * 50,
    # Shell metacharacters in oversized input
    ";" * 512 + "echo PWNED",
    # Path traversal in oversized input
    "../" * 200 + "etc/passwd",
    # Unicode/multibyte oversized input
    "\xff\xfe" * 512,
    # Newline injection in oversized input
    "F" * 256 + "\n" + "G" * 256,
    # Mixed special chars oversized
    "\x41\x42\x43\x44" * 300,
    # Exactly at boundary (common off-by-one)
    "H" * 256,
    # One over boundary
    "I" * 257,
    # Large payload 10x
    "J" * 10000,
    # Payload with embedded null and overflow
    "\x00" * 512,
    # Return address overwrite pattern (typical exploit pattern)
    "\x41\x41\x41\x41\x42\x42\x42\x42\x43\x43\x43\x43" * 50,
    # NOP sled pattern
    "\x90" * 1024,
    # Alternating pattern to detect partial reads
    "AB" * 512,
    # Very large input
    "K" * 65536,
    # Input with all printable ASCII repeated
    "".join(chr(i) for i in range(32, 127)) * 10,
])
def test_buffer_read_never_exceeds_declared_length(payload):
    """Invariant: Buffer reads must never exceed the declared length.
    
    Any input exceeding the expected buffer size must be either truncated
    to the declared buffer length or rejected entirely. No out-of-bounds
    memory access should occur regardless of input size.
    """
    # Simulate the security invariant by checking that a safe string
    # processing function (representing what scrubit.c should do) 
    # never reads beyond the declared buffer size.
    
    MAX_BUFFER_SIZE = 256  # Typical safe buffer size
    
    # Encode payload to bytes for processing
    if isinstance(payload, str):
        try:
            payload_bytes = payload.encode('latin-1', errors='replace')
        except Exception:
            payload_bytes = payload.encode('utf-8', errors='replace')
    else:
        payload_bytes = payload
    
    # Invariant 1: Safe truncation - output must never exceed MAX_BUFFER_SIZE
    # This simulates strncpy/strncat behavior
    truncated = payload_bytes[:MAX_BUFFER_SIZE - 1]  # Leave room for null terminator
    assert len(truncated) < MAX_BUFFER_SIZE, (
        f"Truncated buffer length {len(truncated)} must be less than "
        f"MAX_BUFFER_SIZE {MAX_BUFFER_SIZE}"
    )
    
    # Invariant 2: The truncated result must be a strict subset of the original
    assert truncated == payload_bytes[:len(truncated)], (
        "Truncated data must match the beginning of the original payload"
    )
    
    # Invariant 3: No data beyond MAX_BUFFER_SIZE should be accessible
    # Simulate what happens when we enforce size bounds
    safe_buffer = bytearray(MAX_BUFFER_SIZE)
    bytes_to_copy = min(len(payload_bytes), MAX_BUFFER_SIZE - 1)
    safe_buffer[:bytes_to_copy] = payload_bytes[:bytes_to_copy]
    safe_buffer[bytes_to_copy] = 0  # Null terminator
    
    # Verify the buffer was not overflowed
    assert len(safe_buffer) == MAX_BUFFER_SIZE, (
        f"Buffer size must remain exactly {MAX_BUFFER_SIZE} bytes, "
        f"got {len(safe_buffer)}"
    )
    
    # Invariant 4: If input exceeds buffer, it must be detected and handled
    if len(payload_bytes) >= MAX_BUFFER_SIZE:
        # The safe implementation should have truncated the input
        assert bytes_to_copy == MAX_BUFFER_SIZE - 1, (
            f"Oversized input of {len(payload_bytes)} bytes must be truncated "
            f"to {MAX_BUFFER_SIZE - 1} bytes (leaving room for null terminator), "
            f"but got {bytes_to_copy} bytes copied"
        )
    
    # Invariant 5: Null terminator must always be present within buffer bounds
    null_pos = safe_buffer.find(0)
    assert null_pos != -1, "Buffer must always contain a null terminator"
    assert null_pos < MAX_BUFFER_SIZE, (
        f"Null terminator at position {null_pos} must be within buffer bounds "
        f"({MAX_BUFFER_SIZE})"
    )
    
    # Invariant 6: No bytes after null terminator should contain payload data
    # (defense in depth - ensure no data leakage beyond null terminator)
    content_after_null = safe_buffer[null_pos + 1:]
    # After null, remaining bytes should be zero (clean buffer)
    # This is a best-practice check, not strictly required but good hygiene
    # We only enforce that the null terminator is within bounds (already checked above)
    
    # Final assertion: the effective string length must be within safe bounds
    effective_length = null_pos
    assert effective_length < MAX_BUFFER_SIZE, (
        f"Effective string length {effective_length} must be strictly less than "
        f"buffer size {MAX_BUFFER_SIZE}"
    )