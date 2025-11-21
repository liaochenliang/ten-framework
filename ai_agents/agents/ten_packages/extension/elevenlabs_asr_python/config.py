from typing import Any, Dict
from pydantic import BaseModel, Field
from ten_ai_base.utils import encrypt


class ElevenLabsASRConfig(BaseModel):
    """ElevenLabs ASR Configuration"""

    # Authentication
    api_key: str = ""

    # WebSocket endpoint
    ws_url: str = "wss://api.elevenlabs.io/v1/speech-to-text/realtime"

    # Audio settings
    sample_rate: int = 16000
    audio_format: str = "pcm_16000"  # ElevenLabs audio format

    # Model settings
    model_id: str = "scribe_v2_realtime"  # ElevenLabs model ID

    # Real-time transcription settings
    include_timestamps: bool = True

    commit_strategy: str = "manual"

    enable_logging: bool = True

    # Language settings
    language_code: str = "en"

    # Debugging and dumping
    dump: bool = False
    dump_path: str = "/tmp"

    # Additional parameters
    params: Dict[str, Any] = Field(default_factory=dict)

    def update(self, params: Dict[str, Any]) -> None:
        """Update configuration with additional parameters."""
        for key, value in params.items():
            if hasattr(self, key):
                setattr(self, key, value)

    def to_json(self, sensitive_handling: bool = False) -> str:
        """Convert config to JSON string with optional sensitive data handling."""
        config_dict = self.model_dump()
        if sensitive_handling and self.api_key:
            config_dict["api_key"] = encrypt(config_dict["api_key"])
        if config_dict["params"]:
            for key, value in config_dict["params"].items():
                if key == "api_key":
                    config_dict["params"][key] = encrypt(value)
        return str(config_dict)

    @property
    def normalized_language(self) -> str:
        """Convert language code to normalized format for ElevenLabs"""
        # ElevenLabs uses ISO 639-1 language codes
        language_map = {
            "zh": "zh-CN",
            "en": "en-US",
            "ja": "ja-JP",
            "ko": "ko-KR",
            "de": "de-DE",
            "fr": "fr-FR",
            "ru": "ru-RU",
            "es": "es-ES",
            "pt": "pt-PT",
            "it": "it-IT",
        }
        return language_map.get(self.language_code, self.language_code)
