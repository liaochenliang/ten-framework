# ElevenLabs ASR Python Extension

ElevenLabs Speech Recognition Extension based on ElevenLabs real-time speech-to-text API.

## Features

- Real-time speech recognition
- Multi-language support
- WebSocket connection
- Automatic reconnection mechanism
- Audio data export
- Timestamp support

## Configuration Parameters

### Required Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `api_key` | string | ElevenLabs API key, **must be provided** |

### Optional Parameters

| Parameter | Type | Default Value | Description |
|-----------|------|---------------|-------------|
| `ws_url` | string | `wss://api.elevenlabs.io/v1/speech-to-text/realtime` | WebSocket endpoint URL |
| `sample_rate` | int | `16000` | Audio sample rate |
| `audio_format` | string | `pcm_16000` | Audio format |
| `model_id` | string | `scribe_v2_realtime` | ElevenLabs model ID |
| `language_code` | string | `en` | Language code |
| `include_timestamps` | bool | `true` | Whether to include timestamps |
| `commit_strategy` | string | `manual` | Commit strategy |
| `enable_logging` | bool | `true` | Whether to enable logging |

## Supported Languages

The extension supports the following language codes, automatically converted to ElevenLabs standard format:

- `zh` → `zh-CN` (Chinese)
- `en` → `en-US` (English)
- `ja` → `ja-JP` (Japanese)
- `ko` → `ko-KR` (Korean)
- `de` → `de-DE` (German)
- `fr` → `fr-FR` (French)
- `ru` → `ru-RU` (Russian)
- `es` → `es-ES` (Spanish)
- `pt` → `pt-PT` (Portuguese)
- `it` → `it-IT` (Italian)

## Configuration Examples

### Basic Configuration

```json
{
    "params": {
        "api_key": "your_elevenlabs_api_key_here"
    }
}
```

### Complete Configuration

```json
{
    "params": {
        "api_key": "your_elevenlabs_api_key_here",
        "ws_url": "wss://api.elevenlabs.io/v1/speech-to-text/realtime",
        "sample_rate": 16000,
        "audio_format": "pcm_16000",
        "model_id": "scribe_v2_realtime",
        "language_code": "en",
        "include_timestamps": true,
        "commit_strategy": "manual",
        "enable_logging": true
    }
}
```

### Environment Variable Configuration

Recommended to use environment variables for storing API keys:

```json
{
    "params": {
        "api_key": "${env:ELEVENLABS_ASR_KEY}"
    }
}
```

## Usage

1. **Get API Key**
   - Register for an ElevenLabs account
   - Obtain API key

2. **Configure Extension**
   - Set the required `api_key` in `property.json`
   - Adjust other parameters as needed

3. **Start Extension**
   - Extension will automatically connect to ElevenLabs service
   - Begin real-time speech recognition

## Error Handling

The extension includes comprehensive error handling mechanisms:

- **Connection Errors**: Automatic reconnection mechanism
- **API Key Errors**: Detailed error messages
- **Network Interruptions**: Reconnection manager handling
- **Audio Format Errors**: Format validation and conversion

## Performance Optimization

- **Buffer Strategy**: 10MB audio buffer
- **Audio Format**: Supports PCM 16kHz format
- **Real-time Processing**: Low-latency speech recognition
- **Memory Management**: Automatic audio buffer cleanup

## Dependencies

- `ten_runtime_python` >= 0.11
- `ten_ai_base` >= 0.7
- `pydantic`
- `websockets`

## Important Notes

1. **API Key Security**: Keep your API key secure, do not hardcode it in your source code
2. **Network Requirements**: Requires stable network connection to access ElevenLabs service
3. **Audio Format**: Ensure input audio format matches configuration
4. **Language Support**: Use supported language codes for optimal recognition results

## Troubleshooting

### Common Issues

1. **Connection Failure**
   - Check network connection
   - Verify API key
   - Confirm WebSocket URL is correct

2. **Poor Recognition Quality**
   - Check audio quality
   - Adjust sample rate settings
   - Select appropriate language code

3. **High Memory Usage**
   - Disable audio export feature
   - Adjust buffer size
   - Check if audio streams are properly released

### Log Analysis

Enable `enable_logging` to view detailed logs:
- Connection status changes
- Recognition result details
- Error messages and reconnection attempts
- Performance metrics statistics
