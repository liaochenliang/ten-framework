import asyncio
import inspect



class AudioBufferManager:
    """
    Manages audio data buffering with fixed threshold.

    Features:
    - Fixed threshold (default: 1280 bytes)
    - Automatic buffer management
    - Buffer flushing on demand
    - Support for both sync and async callbacks
    - Detailed logging for monitoring and debugging
    """

    def __init__(self, threshold: int = 1280, send_callback: callable = None, ten_env=None):
        """
        Initialize the audio buffer processor.

        Args:
            threshold (int): Length threshold for sending data chunks (bytes). Default is 1280.
                             Must be a positive integer.
            send_callback (callable, optional): An async callback function (async def) called when a data chunk is ready to send.
                                                It should accept a bytes parameter (the data chunk to send).
                                                If not provided, add_data and flush methods will return a list of sent chunks.
                                                If provided, it must be a coroutine function.
            ten_env: Ten environment for logging purposes.
        """
        if not isinstance(threshold, int) or threshold <= 0:
            raise ValueError("Threshold must be a positive integer.")

        self._buffer = bytearray()
        self._threshold = threshold
        self._send_callback = send_callback
        self._sending_enabled = True  # New status flag, default is to allow sending
        self.ten_env = ten_env

        if self._send_callback and not inspect.iscoroutinefunction(self._send_callback):
            raise TypeError("send_callback must be an async function (a coroutine function) for AsyncAudioBufferProcessor.")

        if self.ten_env:
            self.ten_env.log_debug(f"AsyncAudioBufferProcessor initialized with threshold: {self._threshold} bytes.")
            self.ten_env.log_debug(f"Initial sending state: {'Enabled' if self._sending_enabled else 'Disabled'}")

    def enable_sending(self):
        """Enable sending functionality."""
        self._sending_enabled = True
        if self.ten_env:
            self.ten_env.log_debug("Sending enabled.")

    def disable_sending(self):
        """Disable sending functionality. New add_data calls will not trigger sending, but flush will still send all remaining data."""
        self._sending_enabled = False
        if self.ten_env:
            self.ten_env.log_debug("Sending disabled.")

    def is_sending_enabled(self) -> bool:
        """Check if sending functionality is enabled."""
        return self._sending_enabled

    async def _process_buffer(self, force_send: bool = False) -> list[bytes]:
        """
        Internal async method: Check buffer and process (send) data chunks that reach the threshold.
        Args:
            force_send (bool): If True, ignore _sending_enabled status and force sending.
                               Typically used for flush operations.
        """
        sent_chunks = []
        while len(self._buffer) >= self._threshold:
            chunk_to_send = bytes(self._buffer[:self._threshold])
            del self._buffer[:self._threshold]

            if (self._sending_enabled or force_send) and self._send_callback:
                # Call async callback function
                await self._send_callback(chunk_to_send)
                if self.ten_env:
                    self.ten_env.log_debug(f"[Internal] Sent a chunk of {len(chunk_to_send)} bytes. Remaining buffer: {len(self._buffer)} bytes.")
            elif not self._send_callback:
                sent_chunks.append(chunk_to_send)
                if self.ten_env:
                    self.ten_env.log_debug(f"[Internal] Returned a chunk of {len(chunk_to_send)} bytes. Remaining buffer: {len(self._buffer)} bytes.")
            else:
                # If sending is disabled and not forced, the chunk is processed (removed from buffer) but not sent via callback
                if self.ten_env:
                    self.ten_env.log_debug(f"[Internal] Discarded a chunk of {len(chunk_to_send)} bytes (sending disabled). Remaining buffer: {len(self._buffer)} bytes.")

        return sent_chunks

    async def add_data(self, new_audio_chunk: bytes) -> list[bytes]:
        """
        Add new audio data to the buffer. This is an async method.
        If sending is disabled, data is still added to the buffer but sending is not triggered.

        Args:
            new_audio_chunk (bytes): New audio data chunk (bytes type).

        Returns:
            list[bytes]: If send_callback is not provided, returns a list of all data chunks sent in this operation.
                         If send_callback is provided, returns an empty list.
        """
        if not isinstance(new_audio_chunk, (bytes, bytearray)):
            raise TypeError("new_audio_chunk must be bytes or bytearray.")

        self._buffer.extend(new_audio_chunk)
        if self.ten_env:
            self.ten_env.log_debug(f"Added {len(new_audio_chunk)} bytes. Current buffer size: {len(self._buffer)} bytes.")

        # Call async internal processing method, no forced sending (controlled by _sending_enabled)
        return await self._process_buffer(force_send=False)

    async def flush(self) -> list[bytes]:
        """
        Empty all remaining data in the buffer, even if they haven't reached the threshold.
        This operation forces sending of all data, regardless of _sending_enabled status.
        This is typically called when an audio stream ends. This is an async method.

        Returns:
            list[bytes]: If send_callback is not provided, returns a list of all data chunks sent in this operation.
                         If send_callback is provided, returns an empty list.
        """
        if self.ten_env:
            self.ten_env.log_debug("Flushing buffer...")
        # First process all possible complete data chunks, force sending
        sent_chunks = await self._process_buffer(force_send=True)

        # Process remaining data below threshold, force sending
        if self._buffer:
            remaining_chunk = bytes(self._buffer)
            if self._send_callback:
                await self._send_callback(remaining_chunk)
            else:
                sent_chunks.append(remaining_chunk)

            if self.ten_env:
                self.ten_env.log_debug(f"Flushing remaining {len(remaining_chunk)} bytes.")
            self._buffer.clear()
        else:
            if self.ten_env:
                self.ten_env.log_debug("Buffer is empty, nothing to flush.")

        return sent_chunks

    def get_current_buffer_size(self) -> int:
        """
        Get the number of bytes in the current buffer.
        """
        return len(self._buffer)
