import 'dart:async';

import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_local_notifications/flutter_local_notifications.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'package:supabase_flutter/supabase_flutter.dart';

const String supabaseUrl = 'https://yjjpgvsycxlaqubvedoa.supabase.co';
const String supabaseAnonKey = 'sb_publishable_mf_gFIoOv0-tIu9bhe6fzw_-CYi7BTd';
int riskClass = 0;
String riskLabel = "UNKNOWN";

// ─── Brand palette (matches the HTML splash) ───────────────────────────────
class AppColors {
  static const bg = Color(0xFF0C1A10); // very dark green-black
  static const surface = Color(0xFF152A1C); // card surface
  static const surface2 = Color(0xFF1C3526); // slightly lighter surface
  static const border = Color(0xFF2A3D2E); // subtle border
  static const accent = Color(0xFF4ADE80); // bright green
  static const accentDim = Color(0xFF2EA05A); // mid green
  static const textHigh = Color(0xFFE4F5E9); // near-white
  static const textMid = Color(0xFFB4E6C3); // medium
  static const textLow = Color(0xFF4A6B52); // muted
  static const gridLine = Color(0x0D4ADE80); // 5% green for grid
  static const healthy = Color(0xFF4ADE80);
  static const moderate = Color(0xFFFBBF24);
  static const high = Color(0xFFF87171);
}

// ─── Theme ──────────────────────────────────────────────────────────────────
ThemeData buildAppTheme(TargetPlatform platform) {
  final isDesktop =
      platform == TargetPlatform.windows ||
      platform == TargetPlatform.macOS ||
      platform == TargetPlatform.linux;

  final base = ThemeData(
    brightness: Brightness.dark,
    scaffoldBackgroundColor: AppColors.bg,
    colorScheme: const ColorScheme.dark(
      primary: AppColors.accent,
      onPrimary: AppColors.bg,
      secondary: AppColors.accentDim,
      surface: AppColors.surface,
      onSurface: AppColors.textHigh,
      outline: AppColors.border,
    ),
    platform: platform,
    useMaterial3: true,
    visualDensity: isDesktop ? VisualDensity.compact : VisualDensity.standard,
  );

  final outfitText = GoogleFonts.outfitTextTheme(
    base.textTheme,
  ).apply(bodyColor: AppColors.textMid, displayColor: AppColors.textHigh);

  return base.copyWith(
    textTheme: outfitText,

    // AppBar
    appBarTheme: AppBarTheme(
      backgroundColor: AppColors.bg,
      surfaceTintColor: Colors.transparent,
      elevation: 0,
      titleTextStyle: GoogleFonts.outfit(
        color: AppColors.textHigh,
        fontSize: 18,
        fontWeight: FontWeight.w600,
        letterSpacing: -0.3,
      ),
      iconTheme: const IconThemeData(color: AppColors.textMid),
    ),

    // Cards
    cardTheme: CardThemeData(
      color: AppColors.surface,
      elevation: 0,
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(16),
        side: const BorderSide(color: AppColors.border),
      ),
    ),

    // Dropdowns
    dropdownMenuTheme: DropdownMenuThemeData(
      textStyle: GoogleFonts.outfit(color: AppColors.textMid, fontSize: 14),
      inputDecorationTheme: InputDecorationTheme(
        filled: true,
        fillColor: AppColors.surface,
        labelStyle: GoogleFonts.outfit(color: AppColors.textLow, fontSize: 13),
        border: OutlineInputBorder(
          borderRadius: BorderRadius.circular(10),
          borderSide: const BorderSide(color: AppColors.border),
        ),
        enabledBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(10),
          borderSide: const BorderSide(color: AppColors.border),
        ),
        focusedBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(10),
          borderSide: const BorderSide(color: AppColors.accent, width: 1.5),
        ),
      ),
    ),

    inputDecorationTheme: InputDecorationTheme(
      filled: true,
      fillColor: AppColors.surface,
      labelStyle: GoogleFonts.outfit(color: AppColors.textLow, fontSize: 13),
      border: OutlineInputBorder(
        borderRadius: BorderRadius.circular(10),
        borderSide: const BorderSide(color: AppColors.border),
      ),
      enabledBorder: OutlineInputBorder(
        borderRadius: BorderRadius.circular(10),
        borderSide: const BorderSide(color: AppColors.border),
      ),
      focusedBorder: OutlineInputBorder(
        borderRadius: BorderRadius.circular(10),
        borderSide: const BorderSide(color: AppColors.accent, width: 1.5),
      ),
    ),

    // Chips
    chipTheme: ChipThemeData(
      backgroundColor: AppColors.surface,
      selectedColor: const Color(0xFF1C3D28),
      side: const BorderSide(color: AppColors.border),
      labelStyle: GoogleFonts.outfit(color: AppColors.textMid, fontSize: 13),
      secondaryLabelStyle: GoogleFonts.outfit(
        color: AppColors.accent,
        fontSize: 13,
        fontWeight: FontWeight.w500,
      ),
      checkmarkColor: AppColors.accent,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20)),
    ),

    // Filled buttons
    filledButtonTheme: FilledButtonThemeData(
      style: FilledButton.styleFrom(
        backgroundColor: AppColors.accent,
        foregroundColor: AppColors.bg,
        textStyle: GoogleFonts.outfit(
          fontWeight: FontWeight.w600,
          fontSize: 14,
        ),
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(10)),
        padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 14),
      ),
    ),

    // Outlined buttons
    outlinedButtonTheme: OutlinedButtonThemeData(
      style: OutlinedButton.styleFrom(
        foregroundColor: AppColors.textMid,
        side: const BorderSide(color: AppColors.border),
        textStyle: GoogleFonts.outfit(
          fontWeight: FontWeight.w400,
          fontSize: 14,
        ),
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(10)),
        padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 14),
      ),
    ),

    dividerTheme: const DividerThemeData(
      color: Color(0xFF2A3D2E),
      thickness: 1,
    ),
  );
}

// ─── Entry point ─────────────────────────────────────────────────────────────
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Supabase.initialize(url: supabaseUrl, anonKey: supabaseAnonKey);
  await AppSettings.load();
  await NotificationService.init();
  runApp(const MyApp());
}

final supabase = Supabase.instance.client;

// ─── App settings (persisted) ────────────────────────────────────────────────
class AppSettings {
  static bool notificationsEnabled = true;
  static bool stressUpdatesEnabled = true;
  static SharedPreferences? _prefs;

  static Future<void> load() async {
    _prefs = await SharedPreferences.getInstance();
    notificationsEnabled = _prefs?.getBool('notifications_enabled') ?? true;
    stressUpdatesEnabled = _prefs?.getBool('stress_updates_enabled') ?? true;
  }

  static Future<void> setNotificationsEnabled(bool value) async {
    notificationsEnabled = value;
    await _prefs?.setBool('notifications_enabled', value);
  }

  static Future<void> setStressUpdatesEnabled(bool value) async {
    stressUpdatesEnabled = value;
    await _prefs?.setBool('stress_updates_enabled', value);
  }
}

// ─── Local notification service ──────────────────────────────────────────────
class NotificationService {
  static final _plugin = FlutterLocalNotificationsPlugin();
  static bool _initialized = false;
  static String? _lastNotifiedKey;

  static Future<void> init() async {
    if (_initialized) return;
    const android = AndroidInitializationSettings('@mipmap/ic_launcher');
    const ios = DarwinInitializationSettings(
      requestAlertPermission: true,
      requestBadgePermission: true,
      requestSoundPermission: true,
    );
    await _plugin.initialize(
      const InitializationSettings(android: android, iOS: ios),
    );
    _initialized = true;
  }

  static Future<void> maybeNotify(
    String plantName,
    int risk,
    List<String> actions,
  ) async {
    if (!AppSettings.notificationsEnabled) return;
    if (risk == 0 || actions.isEmpty) return;

    final key = '$plantName:$risk:${actions.join(",")}';
    if (key == _lastNotifiedKey) return;
    _lastNotifiedKey = key;

    final riskWord = risk == 2 ? 'High risk' : 'Moderate risk';
    final body = '$riskWord detected — ${actions.join(", ")}';

    const androidDetails = AndroidNotificationDetails(
      'plant_alerts',
      'Plant Alerts',
      channelDescription: 'Alerts when your plant needs attention',
      importance: Importance.high,
      priority: Priority.high,
    );
    const iosDetails = DarwinNotificationDetails();
    await _plugin.show(
      0,
      'PlantHealth: $plantName',
      body,
      const NotificationDetails(android: androidDetails, iOS: iosDetails),
    );
  }

  static Future<void> sendStressUpdate(String plantName, int risk) async {
    if (!AppSettings.stressUpdatesEnabled) return;

    final stressLabel = switch (risk) {
      0 => 'Healthy — no stress detected',
      1 => 'Moderate stress — check your plant soon',
      2 => 'High stress — your plant needs attention',
      _ => 'Status unknown',
    };

    const androidDetails = AndroidNotificationDetails(
      'stress_updates',
      'Stress Updates',
      channelDescription: 'Periodic plant stress level updates every 3 hours',
      importance: Importance.defaultImportance,
      priority: Priority.defaultPriority,
    );
    const iosDetails = DarwinNotificationDetails();
    await _plugin.show(
      1,
      'Stress update: $plantName',
      stressLabel,
      const NotificationDetails(android: androidDetails, iOS: iosDetails),
    );
  }
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      theme: buildAppTheme(defaultTargetPlatform),
      home: const SplashScreen(),
    );
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SPLASH SCREEN
// ═══════════════════════════════════════════════════════════════════════════════

class SplashScreen extends StatefulWidget {
  const SplashScreen({super.key});

  @override
  State<SplashScreen> createState() => _SplashScreenState();
}

class _SplashScreenState extends State<SplashScreen>
    with TickerProviderStateMixin {
  // Animation controllers
  late final AnimationController _gridCtrl;
  late final AnimationController _ringCtrl;
  late final AnimationController _iconCtrl;
  late final AnimationController _fadeCtrl;
  late final AnimationController _barCtrl;
  late final AnimationController _pulseCtrl;
  late final AnimationController _dotCtrl;

  // Animations
  late final Animation<double> _gridOpacity;
  late final Animation<double> _ring1Scale;
  late final Animation<double> _ring1Opacity;
  late final Animation<double> _ring2Scale;
  late final Animation<double> _ring2Opacity;
  late final Animation<double> _iconScale;
  late final Animation<double> _iconOpacity;
  late final Animation<double> _iconY;
  late final Animation<double> _titleOpacity;
  late final Animation<double> _titleY;
  late final Animation<double> _taglineOpacity;
  late final Animation<double> _pillsOpacity;
  late final Animation<double> _pillsY;
  late final Animation<double> _barProgress;
  late final Animation<double> _glowPulse;
  late final Animation<double> _dotScale;

  @override
  void initState() {
    super.initState();

    // Grid fade in (0–2500ms)
    _gridCtrl = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 2500),
    );
    _gridOpacity = Tween<double>(
      begin: 0,
      end: 1,
    ).animate(CurvedAnimation(parent: _gridCtrl, curve: Curves.easeIn));

    // Rings burst in then pulse
    _ringCtrl = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 800),
    );
    _ring1Scale = Tween<double>(begin: 0.7, end: 1.0).animate(
      CurvedAnimation(
        parent: _ringCtrl,
        curve: const Interval(0.0, 0.6, curve: Curves.easeOut),
      ),
    );
    _ring1Opacity = Tween<double>(begin: 0, end: 1).animate(
      CurvedAnimation(
        parent: _ringCtrl,
        curve: const Interval(0.0, 0.6, curve: Curves.easeOut),
      ),
    );
    _ring2Scale = Tween<double>(begin: 0.7, end: 1.0).animate(
      CurvedAnimation(
        parent: _ringCtrl,
        curve: const Interval(0.2, 0.8, curve: Curves.easeOut),
      ),
    );
    _ring2Opacity = Tween<double>(begin: 0, end: 1).animate(
      CurvedAnimation(
        parent: _ringCtrl,
        curve: const Interval(0.2, 0.8, curve: Curves.easeOut),
      ),
    );

    // Icon pop in (delay 500ms)
    _iconCtrl = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 700),
    );
    _iconScale = Tween<double>(
      begin: 0.3,
      end: 1.0,
    ).animate(CurvedAnimation(parent: _iconCtrl, curve: Curves.elasticOut));
    _iconOpacity = Tween<double>(begin: 0, end: 1).animate(
      CurvedAnimation(parent: _iconCtrl, curve: const Interval(0, 0.4)),
    );
    _iconY = Tween<double>(
      begin: 16,
      end: 0,
    ).animate(CurvedAnimation(parent: _iconCtrl, curve: Curves.easeOut));

    // Title + tagline + pills (staggered via single controller)
    _fadeCtrl = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 800),
    );
    _titleOpacity = Tween<double>(begin: 0, end: 1).animate(
      CurvedAnimation(
        parent: _fadeCtrl,
        curve: const Interval(0.0, 0.6, curve: Curves.easeOut),
      ),
    );
    _titleY = Tween<double>(begin: 12, end: 0).animate(
      CurvedAnimation(
        parent: _fadeCtrl,
        curve: const Interval(0.0, 0.6, curve: Curves.easeOut),
      ),
    );
    _taglineOpacity = Tween<double>(begin: 0, end: 1).animate(
      CurvedAnimation(
        parent: _fadeCtrl,
        curve: const Interval(0.25, 0.85, curve: Curves.easeOut),
      ),
    );
    _pillsOpacity = Tween<double>(begin: 0, end: 1).animate(
      CurvedAnimation(
        parent: _fadeCtrl,
        curve: const Interval(0.5, 1.0, curve: Curves.easeOut),
      ),
    );
    _pillsY = Tween<double>(begin: 12, end: 0).animate(
      CurvedAnimation(
        parent: _fadeCtrl,
        curve: const Interval(0.5, 1.0, curve: Curves.easeOut),
      ),
    );

    // Progress bar
    _barCtrl = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 2600),
    );
    _barProgress = Tween<double>(
      begin: 0,
      end: 1,
    ).animate(CurvedAnimation(parent: _barCtrl, curve: Curves.easeInOut));

    // Radial glow pulse (continuous)
    _pulseCtrl = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 3200),
    )..repeat(reverse: true);
    _glowPulse = Tween<double>(
      begin: 0.9,
      end: 1.1,
    ).animate(CurvedAnimation(parent: _pulseCtrl, curve: Curves.easeInOut));

    // Status dot ping
    _dotCtrl = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 2200),
    )..repeat();
    _dotScale = Tween<double>(
      begin: 1.0,
      end: 2.2,
    ).animate(CurvedAnimation(parent: _dotCtrl, curve: Curves.easeOut));

    // Kick off the sequence
    _runSequence();
  }

  Future<void> _runSequence() async {
    _gridCtrl.forward();
    await Future.delayed(const Duration(milliseconds: 200));
    _ringCtrl.forward();
    await Future.delayed(const Duration(milliseconds: 300));
    _iconCtrl.forward();
    await Future.delayed(const Duration(milliseconds: 400));
    _fadeCtrl.forward();
    await Future.delayed(const Duration(milliseconds: 300));
    _barCtrl.forward();

    // Navigate after bar completes
    _barCtrl.addStatusListener((status) {
      if (status == AnimationStatus.completed) {
        _navigateToDashboard();
      }
    });
  }

  void _navigateToDashboard() {
    if (!mounted) return;
    Navigator.of(context).pushReplacement(
      PageRouteBuilder(
        pageBuilder: (_, __, ___) => const Dashboard(),
        transitionsBuilder: (_, anim, __, child) =>
            FadeTransition(opacity: anim, child: child),
        transitionDuration: const Duration(milliseconds: 600),
      ),
    );
  }

  @override
  void dispose() {
    _gridCtrl.dispose();
    _ringCtrl.dispose();
    _iconCtrl.dispose();
    _fadeCtrl.dispose();
    _barCtrl.dispose();
    _pulseCtrl.dispose();
    _dotCtrl.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final size = MediaQuery.sizeOf(context);

    return Scaffold(
      backgroundColor: AppColors.bg,
      body: Stack(
        children: [
          // ── Grid background ──────────────────────────────────────────
          AnimatedBuilder(
            animation: _gridOpacity,
            builder: (_, __) => Opacity(
              opacity: _gridOpacity.value,
              child: CustomPaint(size: size, painter: _GridPainter()),
            ),
          ),

          // ── Radial glow ──────────────────────────────────────────────
          Positioned.fill(
            child: AnimatedBuilder(
              animation: _glowPulse,
              builder: (_, __) => Center(
                child: Transform.scale(
                  scale: _glowPulse.value,
                  child: Container(
                    width: size.width * 0.7,
                    height: size.width * 0.7,
                    decoration: const BoxDecoration(
                      shape: BoxShape.circle,
                      gradient: RadialGradient(
                        colors: [Color(0x242EA05A), Colors.transparent],
                      ),
                    ),
                  ),
                ),
              ),
            ),
          ),

          // ── Concentric rings ─────────────────────────────────────────
          Positioned.fill(
            child: AnimatedBuilder(
              animation: _ringCtrl,
              builder: (_, __) => Center(
                child: Stack(
                  alignment: Alignment.center,
                  children: [
                    // outer ring
                    Opacity(
                      opacity: _ring2Opacity.value,
                      child: Transform.scale(
                        scale: _ring2Scale.value,
                        child: Container(
                          width: size.width * 0.72,
                          height: size.width * 0.72,
                          decoration: BoxDecoration(
                            shape: BoxShape.circle,
                            border: Border.all(
                              color: const Color(0x124ADE80),
                              width: 1,
                            ),
                          ),
                        ),
                      ),
                    ),
                    // inner ring
                    Opacity(
                      opacity: _ring1Opacity.value,
                      child: Transform.scale(
                        scale: _ring1Scale.value,
                        child: Container(
                          width: size.width * 0.52,
                          height: size.width * 0.52,
                          decoration: BoxDecoration(
                            shape: BoxShape.circle,
                            border: Border.all(
                              color: const Color(0x264ADE80),
                              width: 1,
                            ),
                          ),
                        ),
                      ),
                    ),
                  ],
                ),
              ),
            ),
          ),

          // ── Status dot (top right) ────────────────────────────────────
          Positioned(
            top: 56,
            right: 28,
            child: AnimatedBuilder(
              animation: _dotCtrl,
              builder: (_, __) => Stack(
                alignment: Alignment.center,
                children: [
                  Opacity(
                    opacity: 1 - _dotCtrl.value,
                    child: Transform.scale(
                      scale: _dotScale.value,
                      child: Container(
                        width: 8,
                        height: 8,
                        decoration: const BoxDecoration(
                          shape: BoxShape.circle,
                          color: Color(0x554ADE80),
                        ),
                      ),
                    ),
                  ),
                  Container(
                    width: 8,
                    height: 8,
                    decoration: const BoxDecoration(
                      shape: BoxShape.circle,
                      color: AppColors.accent,
                    ),
                  ),
                ],
              ),
            ),
          ),

          // ── Main content ─────────────────────────────────────────────
          Positioned.fill(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                // Icon
                AnimatedBuilder(
                  animation: _iconCtrl,
                  builder: (_, __) => Opacity(
                    opacity: _iconOpacity.value,
                    child: Transform.translate(
                      offset: Offset(0, _iconY.value),
                      child: Transform.scale(
                        scale: _iconScale.value,
                        child: const _PlantIconBox(),
                      ),
                    ),
                  ),
                ),
                const SizedBox(height: 28),

                // Title
                AnimatedBuilder(
                  animation: _fadeCtrl,
                  builder: (_, __) => Opacity(
                    opacity: _titleOpacity.value,
                    child: Transform.translate(
                      offset: Offset(0, _titleY.value),
                      child: Column(
                        children: [
                          RichText(
                            text: TextSpan(
                              children: [
                                TextSpan(
                                  text: 'Plant',
                                  style: GoogleFonts.outfit(
                                    fontSize: 38,
                                    fontWeight: FontWeight.w600,
                                    color: AppColors.textHigh,
                                    letterSpacing: -1.0,
                                  ),
                                ),
                                TextSpan(
                                  text: 'Health',
                                  style: GoogleFonts.outfit(
                                    fontSize: 38,
                                    fontWeight: FontWeight.w300,
                                    color: AppColors.accent,
                                    letterSpacing: -1.0,
                                  ),
                                ),
                              ],
                            ),
                          ),
                          const SizedBox(height: 2),
                          Text(
                            'Monitor',
                            style: GoogleFonts.outfit(
                              fontSize: 20,
                              fontWeight: FontWeight.w300,
                              color: const Color(0x99E4F5E9),
                              letterSpacing: -0.3,
                            ),
                          ),
                        ],
                      ),
                    ),
                  ),
                ),
                const SizedBox(height: 12),

                // Tagline
                AnimatedBuilder(
                  animation: _fadeCtrl,
                  builder: (_, __) => Opacity(
                    opacity: _taglineOpacity.value,
                    child: Text(
                      'SMART PLANT COMPANION',
                      style: GoogleFonts.outfit(
                        fontSize: 11,
                        fontWeight: FontWeight.w400,
                        color: const Color(0x73B4E6C3),
                        letterSpacing: 3.2,
                      ),
                    ),
                  ),
                ),
                const SizedBox(height: 32),

                // Feature pills
                AnimatedBuilder(
                  animation: _fadeCtrl,
                  builder: (_, __) => Opacity(
                    opacity: _pillsOpacity.value,
                    child: Transform.translate(
                      offset: Offset(0, _pillsY.value),
                      child: Wrap(
                        spacing: 8,
                        runSpacing: 8,
                        alignment: WrapAlignment.center,
                        children: const [
                          _FeaturePill('Live data'),
                          _FeaturePill('Preferences'),
                          _FeaturePill('Suggestions'),
                        ],
                      ),
                    ),
                  ),
                ),
                const SizedBox(height: 44),

                // Progress bar
                AnimatedBuilder(
                  animation: _barCtrl,
                  builder: (_, __) => Opacity(
                    opacity: _barProgress.value > 0 ? 1 : 0,
                    child: SizedBox(
                      width: 160,
                      child: Column(
                        children: [
                          Container(
                            height: 2,
                            decoration: BoxDecoration(
                              color: const Color(0x12FFFFFF),
                              borderRadius: BorderRadius.circular(2),
                            ),
                            child: FractionallySizedBox(
                              alignment: Alignment.centerLeft,
                              widthFactor: _barProgress.value,
                              child: Container(
                                decoration: BoxDecoration(
                                  color: AppColors.accent,
                                  borderRadius: BorderRadius.circular(2),
                                ),
                              ),
                            ),
                          ),
                          const SizedBox(height: 10),
                          Text(
                            'Initializing dashboard...',
                            style: GoogleFonts.outfit(
                              fontSize: 10,
                              color: const Color(0x4DB4E6C3),
                              letterSpacing: 0.8,
                            ),
                          ),
                        ],
                      ),
                    ),
                  ),
                ),
              ],
            ),
          ),

          // ── Version tag ──────────────────────────────────────────────
          Positioned(
            bottom: 48,
            left: 0,
            right: 0,
            child: Text(
              'v2.1.0',
              textAlign: TextAlign.center,
              style: GoogleFonts.outfit(
                fontSize: 11,
                color: const Color(0x2EB4E6C3),
                letterSpacing: 0.5,
              ),
            ),
          ),

          // ── Home bar (iOS look) ───────────────────────────────────────
          Positioned(
            bottom: 18,
            left: 0,
            right: 0,
            child: Center(
              child: Container(
                width: 100,
                height: 3,
                decoration: BoxDecoration(
                  color: const Color(0x26FFFFFF),
                  borderRadius: BorderRadius.circular(3),
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }
}

// ─── Grid painter ────────────────────────────────────────────────────────────
class _GridPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    final paint = Paint()
      ..color = AppColors.gridLine
      ..strokeWidth = 1;

    const step = 28.0;
    for (double x = 0; x <= size.width; x += step) {
      canvas.drawLine(Offset(x, 0), Offset(x, size.height), paint);
    }
    for (double y = 0; y <= size.height; y += step) {
      canvas.drawLine(Offset(0, y), Offset(size.width, y), paint);
    }
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}

// ─── Plant icon box ──────────────────────────────────────────────────────────
class _PlantIconBox extends StatelessWidget {
  const _PlantIconBox();

  @override
  Widget build(BuildContext context) {
    return Container(
      width: 80,
      height: 80,
      decoration: BoxDecoration(
        color: AppColors.surface,
        borderRadius: BorderRadius.circular(20),
        border: Border.all(color: const Color(0x4D4ADE80)),
      ),
      child: Center(child: _PlantSvgIcon()),
    );
  }
}

class _PlantSvgIcon extends StatelessWidget {
  const _PlantSvgIcon();

  @override
  Widget build(BuildContext context) {
    return CustomPaint(size: const Size(44, 44), painter: _PlantIconPainter());
  }
}

class _PlantIconPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    final accent = AppColors.accent;
    final accentFill = const Color(0x384ADE80);
    final accentFill2 = const Color(0x284ADE80);
    final accentFill3 = const Color(0x474ADE80);

    final strokePaint = Paint()
      ..color = accent
      ..style = PaintingStyle.stroke
      ..strokeWidth = 1.6
      ..strokeCap = StrokeCap.round;

    final fillPaint = Paint()..style = PaintingStyle.fill;

    final cx = size.width / 2;

    // Stem
    canvas.drawLine(
      Offset(cx, size.height * 0.97),
      Offset(cx, size.height * 0.38),
      strokePaint,
    );

    // Left leaf (mid)
    final leftLeafPath = Path()
      ..moveTo(cx, size.height * 0.62)
      ..cubicTo(
        cx - 8,
        size.height * 0.58,
        cx - 12,
        size.height * 0.47,
        cx - 11,
        size.height * 0.38,
      )
      ..cubicTo(
        cx - 4,
        size.height * 0.42,
        cx + 2,
        size.height * 0.52,
        cx,
        size.height * 0.62,
      )
      ..close();
    fillPaint.color = accentFill;
    canvas.drawPath(leftLeafPath, fillPaint);
    canvas.drawPath(leftLeafPath, strokePaint..strokeWidth = 1.4);

    // Right leaf (upper)
    final rightLeafPath = Path()
      ..moveTo(cx, size.height * 0.48)
      ..cubicTo(
        cx + 8,
        size.height * 0.43,
        cx + 12,
        size.height * 0.32,
        cx + 10,
        size.height * 0.23,
      )
      ..cubicTo(
        cx + 3,
        size.height * 0.28,
        cx - 3,
        size.height * 0.38,
        cx,
        size.height * 0.48,
      )
      ..close();
    fillPaint.color = accentFill2;
    canvas.drawPath(rightLeafPath, fillPaint);
    canvas.drawPath(rightLeafPath, strokePaint);

    // Top leaf
    final topLeafPath = Path()
      ..moveTo(cx, size.height * 0.35)
      ..cubicTo(
        cx - 4,
        size.height * 0.25,
        cx - 2,
        size.height * 0.1,
        cx + 2,
        size.height * 0.05,
      )
      ..cubicTo(
        cx + 5,
        size.height * 0.12,
        cx + 4,
        size.height * 0.25,
        cx,
        size.height * 0.35,
      )
      ..close();
    fillPaint.color = accentFill3;
    canvas.drawPath(topLeafPath, fillPaint);
    canvas.drawPath(topLeafPath, strokePaint);

    // Soil line
    final soilPaint = Paint()
      ..color = const Color(0xFF1E4D28)
      ..style = PaintingStyle.stroke
      ..strokeWidth = 2.4
      ..strokeCap = StrokeCap.round;
    canvas.drawLine(
      Offset(cx - 10, size.height * 0.97),
      Offset(cx + 10, size.height * 0.97),
      soilPaint,
    );

    // Root dot
    final dotPaint = Paint()
      ..color = accent
      ..style = PaintingStyle.fill;
    canvas.drawCircle(Offset(cx, size.height * 0.97), 1.8, dotPaint);

    // Small sensor circles
    final sensorBorder = Paint()
      ..color = const Color(0x724ADE80)
      ..style = PaintingStyle.stroke
      ..strokeWidth = 0.9;
    final sensorFill = Paint()
      ..color = accent
      ..style = PaintingStyle.fill;

    canvas.drawCircle(Offset(cx + 9, size.height * 0.49), 3.2, sensorBorder);
    canvas.drawCircle(Offset(cx + 9, size.height * 0.49), 1.3, sensorFill);

    canvas.drawCircle(Offset(cx - 7, size.height * 0.65), 2.6, sensorBorder);
    canvas.drawCircle(Offset(cx - 7, size.height * 0.65), 1.0, sensorFill);
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}

// ─── Feature pill ────────────────────────────────────────────────────────────
class _FeaturePill extends StatelessWidget {
  const _FeaturePill(this.label);
  final String label;

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 7),
      decoration: BoxDecoration(
        color: const Color(0x144ADE80),
        border: Border.all(color: const Color(0x334ADE80)),
        borderRadius: BorderRadius.circular(24),
      ),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          Container(
            width: 6,
            height: 6,
            decoration: const BoxDecoration(
              shape: BoxShape.circle,
              color: Color(0xB34ADE80),
            ),
          ),
          const SizedBox(width: 7),
          Text(
            label,
            style: GoogleFonts.outfit(
              fontSize: 11,
              fontWeight: FontWeight.w400,
              color: const Color(0xB3B4E6C3),
              letterSpacing: 0.3,
            ),
          ),
        ],
      ),
    );
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  DASHBOARD
// ═══════════════════════════════════════════════════════════════════════════════

class Dashboard extends StatefulWidget {
  const Dashboard({super.key});

  @override
  State<Dashboard> createState() => _DashboardState();
}

class _DashboardState extends State<Dashboard> {
  List<String> plants = [];
  String? selectedPlant;
  List<DateTime> timestamps = [];
  String recommendationText = "";
  List<String> actions = [];

  final metrics = [
    "soil_moisture_pct",
    "temperature_c",
    "humidity_pct",
    "light_level_pct",
  ];

  List<String> selectedMetrics = ["soil_moisture_pct"];
  String timeRange = "24h";
  Map<String, List<FlSpot>> graphData = {};
  String statusText = "No plant selected...";
  Color statusColor = AppColors.surface;
  Timer? _stressTimer;

  Future<void> loadPlants() async {
    final res = await supabase.from('plant_settings').select('plant_label');
    final list = (res as List).map((e) => e['plant_label'] as String).toList();
    if (!mounted) return;
    setState(() {
      plants = list;
      if (!plants.contains(selectedPlant)) selectedPlant = null;
    });
  }

  Future<void> loadLatestStatus() async {
    if (selectedPlant == null) return;

    final res = await supabase
        .from('plant_readings')
        .select()
        .eq('plant_label', selectedPlant!)
        .order('timestamp', ascending: false)
        .limit(1);

    if (res.isEmpty) return;

    final latest = res[0];
    final int risk = latest['risk_class'] ?? 0;
    actions = [];

    if (latest['action_reduce_temp'] == true ||
        latest['action_reduce_temp'] == "true" ||
        latest['action_reduce_temp'] == 1) {
      actions.add("Reduce temperature");
    }
    if (latest['action_water'] == true ||
        latest['action_water'] == "true" ||
        latest['action_water'] == 1) {
      actions.add("Water the plant");
    }
    if (latest['action_increase_light'] == true ||
        latest['action_increase_light'] == "true" ||
        latest['action_increase_light'] == 1) {
      actions.add("Increase light exposure");
    }

    if (!mounted) return;

    setState(() {
      riskClass = risk;
      switch (risk) {
        case 0:
          riskLabel = "HEALTHY";
          statusColor = AppColors.healthy;
          statusText = "Plant is healthy";
          break;
        case 1:
          riskLabel = "MODERATE RISK";
          statusColor = AppColors.moderate;
          statusText = "Moderate risk detected";
          break;
        case 2:
          riskLabel = "HIGH RISK";
          statusColor = AppColors.high;
          statusText = "High risk detected";
          break;
        default:
          riskLabel = "UNKNOWN";
          statusColor = AppColors.textLow;
          statusText = "No data";
      }
      recommendationText = actions.isEmpty
          ? "No actions required"
          : actions.join("\n");
    });

    if (selectedPlant != null) {
      NotificationService.maybeNotify(selectedPlant!, risk, actions);
    }
  }

  bool isInRange(DateTime t) {
    final now = DateTime.now();
    if (timeRange == "24h") return now.difference(t).inHours <= 24;
    if (timeRange == "7d") return now.difference(t).inDays <= 7;
    return true;
  }

  Future<void> loadGraph() async {
    if (selectedPlant == null) return;

    final response = await supabase
        .from('plant_readings')
        .select()
        .eq('plant_label', selectedPlant!)
        .order('timestamp', ascending: true);

    final List<Map<String, dynamic>> data = List<Map<String, dynamic>>.from(
      response,
    );
    final Map<String, List<FlSpot>> temp = {};
    final List<DateTime> loadedTimestamps = [];

    final filtered = data.where((row) {
      final t = DateTime.parse(row['timestamp']);
      return isInRange(t);
    }).toList();

    for (int i = 0; i < filtered.length; i++) {
      final row = filtered[i];
      final time = DateTime.parse(row['timestamp']);
      loadedTimestamps.add(time);
      for (final m in selectedMetrics) {
        final value = (row[m] ?? 0).toDouble().clamp(0.0, 100.0);
        temp.putIfAbsent(m, () => []);
        temp[m]!.add(FlSpot(i.toDouble(), value));
      }
    }

    if (!mounted) return;
    setState(() {
      timestamps = loadedTimestamps;
      graphData = temp;
    });
  }

  Color colorFor(String metric) {
    switch (metric) {
      case "soil_moisture_pct":
        return AppColors.accent;
      case "temperature_c":
        return AppColors.high;
      case "humidity_pct":
        return const Color(0xFF60A5FA);
      case "light_level_pct":
        return const Color(0xFFFBBF24);
      default:
        return AppColors.textMid;
    }
  }

  String label(String m) {
    switch (m) {
      case "soil_moisture_pct":
        return "Soil Moisture";
      case "temperature_c":
        return "Temperature";
      case "humidity_pct":
        return "Humidity";
      case "light_level_pct":
        return "Light";
      default:
        return m;
    }
  }

  @override
  void initState() {
    super.initState();
    loadPlants().then((_) => loadGraph());
    loadLatestStatus();
    _stressTimer = Timer.periodic(const Duration(hours: 3), (_) async {
      if (selectedPlant == null) return;
      final res = await supabase
          .from('plant_readings')
          .select('risk_class')
          .eq('plant_label', selectedPlant!)
          .order('timestamp', ascending: false)
          .limit(1);
      if (res.isNotEmpty) {
        final risk = (res[0]['risk_class'] ?? 0) as int;
        NotificationService.sendStressUpdate(selectedPlant!, risk);
      }
    });
  }

  @override
  void dispose() {
    _stressTimer?.cancel();
    super.dispose();
  }

  Future<void> openAppSettings() async {
    await Navigator.of(context).push(
      MaterialPageRoute(builder: (_) => const AppSettingsPage()),
    );
    if (mounted) setState(() {});
  }

  Future<void> openProfileSettings() async {
    final selected = await Navigator.of(context).push<String>(
      MaterialPageRoute(
        builder: (_) =>
            ProfileSettingsPage(plants: plants, selectedPlant: selectedPlant),
      ),
    );
    if (!mounted || selected == null) return;
    setState(() => selectedPlant = selected);
    await loadGraph();
    await loadLatestStatus();
  }

  Widget buildPlantSelector() {
    final plantValue = plants.contains(selectedPlant) ? selectedPlant : null;
    return DropdownButtonFormField<String>(
      key: ValueKey("dashboard-plant-$plantValue"),
      initialValue: plantValue,
      decoration: const InputDecoration(labelText: "Plant"),
      hint: Text(
        "Select a plant",
        style: GoogleFonts.outfit(color: AppColors.textLow, fontSize: 14),
      ),
      items: plants
          .map(
            (p) => DropdownMenuItem(
              value: p,
              child: Text(
                p,
                style: GoogleFonts.outfit(color: AppColors.textMid),
              ),
            ),
          )
          .toList(),
      onChanged: (v) {
        setState(() => selectedPlant = v);
        loadGraph();
        loadLatestStatus();
      },
    );
  }

  Widget buildTimeRangeSelector() {
    return DropdownButtonFormField<String>(
      key: ValueKey("dashboard-range-$timeRange"),
      initialValue: timeRange,
      decoration: const InputDecoration(labelText: "Time range"),
      items: [
        DropdownMenuItem(
          value: "24h",
          child: Text(
            "Past 24 hours",
            style: GoogleFonts.outfit(color: AppColors.textMid),
          ),
        ),
        DropdownMenuItem(
          value: "7d",
          child: Text(
            "Past 7 days",
            style: GoogleFonts.outfit(color: AppColors.textMid),
          ),
        ),
        DropdownMenuItem(
          value: "all",
          child: Text(
            "All time",
            style: GoogleFonts.outfit(color: AppColors.textMid),
          ),
        ),
      ],
      onChanged: (v) {
        if (v == null) return;
        setState(() => timeRange = v);
        loadGraph();
      },
    );
  }

  Widget buildMetricSelector() {
    return Wrap(
      spacing: 8,
      runSpacing: 8,
      children: metrics.map((m) {
        final selected = selectedMetrics.contains(m);
        return FilterChip(
          label: Text(label(m)),
          selected: selected,
          onSelected: (v) {
            setState(() {
              if (v)
                selectedMetrics.add(m);
              else
                selectedMetrics.remove(m);
            });
            loadGraph();
          },
        );
      }).toList(),
    );
  }

  Widget buildLegend() {
    return Wrap(
      spacing: 14,
      runSpacing: 8,
      children: selectedMetrics.map((m) {
        return Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            Container(
              width: 10,
              height: 10,
              decoration: BoxDecoration(
                color: colorFor(m),
                borderRadius: BorderRadius.circular(3),
              ),
            ),
            const SizedBox(width: 6),
            Text(
              label(m),
              style: GoogleFonts.outfit(fontSize: 12, color: AppColors.textMid),
            ),
          ],
        );
      }).toList(),
    );
  }

  Widget buildChart() {
    if (selectedPlant == null) {
      return Center(
        child: Text(
          "Select a plant to view readings",
          style: GoogleFonts.outfit(color: AppColors.textLow),
        ),
      );
    }
    if (selectedMetrics.isEmpty) {
      return Center(
        child: Text(
          "Select at least one metric",
          style: GoogleFonts.outfit(color: AppColors.textLow),
        ),
      );
    }

    return LineChart(
      LineChartData(
        minY: 0,
        maxY: 100,
        backgroundColor: Colors.transparent,
        lineTouchData: LineTouchData(
          handleBuiltInTouches: true,
          touchTooltipData: LineTouchTooltipData(
            getTooltipColor: (_) => AppColors.surface2,
            getTooltipItems: (spots) {
              final i = spots.first.x.toInt();
              final t = (i >= 0 && i < timestamps.length) ? timestamps[i] : null;
              final dateStr = t != null
                  ? "${t.day}/${t.month}  ${t.hour.toString().padLeft(2, '0')}:${t.minute.toString().padLeft(2, '0')}"
                  : "";
              return List.generate(spots.length, (idx) {
                final spot = spots[idx];
                final metricName = spot.barIndex < selectedMetrics.length
                    ? selectedMetrics[spot.barIndex]
                    : '';
                final isLast = idx == spots.length - 1;
                return LineTooltipItem(
                  "${label(metricName)}: ${spot.y.toStringAsFixed(1)}",
                  GoogleFonts.outfit(color: colorFor(metricName), fontSize: 11),
                  children: isLast && dateStr.isNotEmpty
                      ? [
                          TextSpan(
                            text: '\n$dateStr',
                            style: GoogleFonts.outfit(
                              color: AppColors.textLow,
                              fontSize: 10,
                            ),
                          ),
                        ]
                      : [],
                );
              });
            },
          ),
        ),
        gridData: FlGridData(
          show: true,
          getDrawingHorizontalLine: (_) =>
              const FlLine(color: AppColors.gridLine, strokeWidth: 1),
          getDrawingVerticalLine: (_) =>
              const FlLine(color: AppColors.gridLine, strokeWidth: 1),
        ),
        borderData: FlBorderData(
          show: true,
          border: Border.all(color: AppColors.border),
        ),
        titlesData: FlTitlesData(
          rightTitles: const AxisTitles(
            sideTitles: SideTitles(showTitles: false),
          ),
          topTitles: const AxisTitles(
            sideTitles: SideTitles(showTitles: false),
          ),
          bottomTitles: AxisTitles(
            sideTitles: SideTitles(
              showTitles: true,
              interval: timestamps.isNotEmpty
                  ? (timestamps.length / 5).ceilToDouble()
                  : 1,
              getTitlesWidget: (value, meta) {
                final i = value.toInt();
                if (i < 0 || i >= timestamps.length) return const Text("");
                final t = timestamps[i];
                return Text(
                  "${t.day}/${t.month}",
                  style: GoogleFonts.outfit(
                    fontSize: 10,
                    color: AppColors.textLow,
                  ),
                );
              },
            ),
          ),
          leftTitles: AxisTitles(
            sideTitles: SideTitles(
              showTitles: true,
              reservedSize: 40,
              getTitlesWidget: (value, meta) => Text(
                value.toInt().toString(),
                style: GoogleFonts.outfit(
                  fontSize: 10,
                  color: AppColors.textLow,
                ),
              ),
            ),
          ),
        ),
        lineBarsData: selectedMetrics.map((m) {
          return LineChartBarData(
            spots: graphData[m] ?? [],
            isCurved: true,
            preventCurveOverShooting: true,
            barWidth: 2,
            color: colorFor(m),
            dotData: const FlDotData(show: false),
            belowBarData: BarAreaData(
              show: true,
              color: colorFor(m).withOpacity(0.06),
            ),
          );
        }).toList(),
      ),
    );
  }

  Widget buildStatusCard() {
    final isHealthy = riskClass == 0;
    final borderColor = statusColor.withOpacity(0.4);

    return Container(
      width: double.infinity,
      padding: const EdgeInsets.all(20),
      decoration: BoxDecoration(
        color: AppColors.surface,
        borderRadius: BorderRadius.circular(16),
        border: Border.all(color: borderColor, width: 1),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            children: [
              Container(
                width: 10,
                height: 10,
                decoration: BoxDecoration(
                  shape: BoxShape.circle,
                  color: statusColor,
                  boxShadow: [
                    BoxShadow(
                      color: statusColor.withOpacity(0.5),
                      blurRadius: 6,
                    ),
                  ],
                ),
              ),
              const SizedBox(width: 10),
              Text(
                "STATUS: $riskLabel",
                style: GoogleFonts.outfit(
                  fontSize: 15,
                  fontWeight: FontWeight.w600,
                  color: statusColor,
                  letterSpacing: 0.5,
                ),
              ),
            ],
          ),
          const SizedBox(height: 8),
          Text(
            statusText,
            style: GoogleFonts.outfit(fontSize: 13, color: AppColors.textMid),
          ),
          const SizedBox(height: 16),
          Container(height: 1, color: AppColors.border),
          const SizedBox(height: 14),
          Text(
            "Recommendation",
            style: GoogleFonts.outfit(
              fontSize: 11,
              fontWeight: FontWeight.w500,
              color: AppColors.textLow,
              letterSpacing: 1.5,
            ),
          ),
          const SizedBox(height: 8),
          Text(
            recommendationText,
            style: GoogleFonts.outfit(
              fontSize: 14,
              color: isHealthy ? AppColors.accent : AppColors.textMid,
            ),
          ),
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    final screenWidth = MediaQuery.sizeOf(context).width;
    final isWide = screenWidth >= 700;
    final showSettingsLabel = screenWidth >= 420;
    final contentWidth = isWide ? 1100.0 : double.infinity;

    return Scaffold(
      appBar: AppBar(
        title: Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            const _MiniPlantIcon(),
            const SizedBox(width: 10),
            const Text("PlantHealthMonitor", overflow: TextOverflow.ellipsis),
          ],
        ),
        actions: [
          if (showSettingsLabel) ...[
            FilledButton.icon(
              onPressed: openProfileSettings,
              icon: const Icon(Icons.tune, size: 16),
              label: const Text("Preferences"),
            ),
            const SizedBox(width: 8),
            OutlinedButton.icon(
              onPressed: openAppSettings,
              icon: const Icon(Icons.settings, size: 16),
              label: const Text("Settings"),
            ),
          ] else ...[
            IconButton(
              tooltip: "Plant Preferences",
              onPressed: openProfileSettings,
              icon: const Icon(Icons.tune),
            ),
            IconButton(
              tooltip: "App Settings",
              onPressed: openAppSettings,
              icon: const Icon(Icons.settings),
            ),
          ],
          const SizedBox(width: 8),
        ],
        bottom: PreferredSize(
          preferredSize: const Size.fromHeight(1),
          child: Container(height: 1, color: AppColors.border),
        ),
      ),
      body: Stack(
        children: [
          // Subtle grid background
          Positioned.fill(child: CustomPaint(painter: _GridPainter())),
          SafeArea(
            child: Center(
              child: ConstrainedBox(
                constraints: BoxConstraints(maxWidth: contentWidth),
                child: ListView(
                  padding: const EdgeInsets.all(16),
                  children: [
                    buildPlantSelector(),
                    const SizedBox(height: 12),
                    buildStatusCard(),
                    const SizedBox(height: 12),
                    buildMetricSelector(),
                    const SizedBox(height: 12),
                    buildLegend(),
                    const SizedBox(height: 16),
                    Container(
                      height: isWide ? 420 : 320,
                      padding: const EdgeInsets.all(16),
                      decoration: BoxDecoration(
                        color: AppColors.surface,
                        borderRadius: BorderRadius.circular(16),
                        border: Border.all(color: AppColors.border),
                      ),
                      child: Padding(
                        padding: const EdgeInsets.only(right: 8),
                        child: buildChart(),
                      ),
                    ),
                    const SizedBox(height: 12),
                    buildTimeRangeSelector(),
                    const SizedBox(height: 12),
                    OutlinedButton.icon(
                      onPressed: () => Navigator.of(context).push(
                        MaterialPageRoute(
                          builder: (_) => DataTablePage(
                            plants: plants,
                            initialPlant: selectedPlant,
                          ),
                        ),
                      ),
                      icon: const Icon(Icons.table_chart_outlined, size: 16),
                      label: const Text("View Data Table"),
                    ),
                    const SizedBox(height: 16),
                  ],
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }
}

// ─── Mini plant icon for AppBar ───────────────────────────────────────────────
class _MiniPlantIcon extends StatelessWidget {
  const _MiniPlantIcon();
  @override
  Widget build(BuildContext context) {
    return Container(
      width: 28,
      height: 28,
      decoration: BoxDecoration(
        color: AppColors.surface,
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: const Color(0x334ADE80)),
      ),
      child: Center(
        child: CustomPaint(
          size: const Size(16, 16),
          painter: _PlantIconPainter(),
        ),
      ),
    );
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  APP SETTINGS PAGE
// ═══════════════════════════════════════════════════════════════════════════════

class AppSettingsPage extends StatefulWidget {
  const AppSettingsPage({super.key});

  @override
  State<AppSettingsPage> createState() => _AppSettingsPageState();
}

class _AppSettingsPageState extends State<AppSettingsPage> {
  bool _notifications = AppSettings.notificationsEnabled;
  bool _stressUpdates = AppSettings.stressUpdatesEnabled;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        leading: IconButton(
          tooltip: "Back",
          onPressed: () => Navigator.of(context).pop(),
          icon: const Icon(CupertinoIcons.back),
        ),
        title: const Text("App Settings"),
        bottom: PreferredSize(
          preferredSize: const Size.fromHeight(1),
          child: Container(height: 1, color: AppColors.border),
        ),
      ),
      body: Stack(
        children: [
          Positioned.fill(child: CustomPaint(painter: _GridPainter())),
          SafeArea(
            child: Center(
              child: ConstrainedBox(
                constraints: const BoxConstraints(maxWidth: 720),
                child: ListView(
                  padding: const EdgeInsets.all(16),
                  children: [
                    Text(
                      "NOTIFICATIONS",
                      style: GoogleFonts.outfit(
                        fontSize: 11,
                        fontWeight: FontWeight.w500,
                        color: AppColors.textLow,
                        letterSpacing: 1.8,
                      ),
                    ),
                    const SizedBox(height: 12),
                    Container(
                      padding: const EdgeInsets.symmetric(
                        horizontal: 16,
                        vertical: 4,
                      ),
                      decoration: BoxDecoration(
                        color: AppColors.surface,
                        borderRadius: BorderRadius.circular(16),
                        border: Border.all(color: AppColors.border),
                      ),
                      child: SwitchListTile(
                        contentPadding: EdgeInsets.zero,
                        title: Text(
                          "Plant alerts",
                          style: GoogleFonts.outfit(
                            color: AppColors.textHigh,
                            fontSize: 15,
                          ),
                        ),
                        subtitle: Text(
                          "Get notified when your plant needs watering, light adjustment, or temperature changes.",
                          style: GoogleFonts.outfit(
                            color: AppColors.textLow,
                            fontSize: 12,
                          ),
                        ),
                        value: _notifications,
                        activeThumbColor: AppColors.accent,
                        activeTrackColor: AppColors.accentDim,
                        onChanged: (v) async {
                          setState(() => _notifications = v);
                          await AppSettings.setNotificationsEnabled(v);
                        },
                      ),
                    ),
                    const SizedBox(height: 12),
                    Container(
                      padding: const EdgeInsets.symmetric(
                        horizontal: 16,
                        vertical: 4,
                      ),
                      decoration: BoxDecoration(
                        color: AppColors.surface,
                        borderRadius: BorderRadius.circular(16),
                        border: Border.all(color: AppColors.border),
                      ),
                      child: SwitchListTile(
                        contentPadding: EdgeInsets.zero,
                        title: Text(
                          "Stress level updates",
                          style: GoogleFonts.outfit(
                            color: AppColors.textHigh,
                            fontSize: 15,
                          ),
                        ),
                        subtitle: Text(
                          "Receive a stress level summary every 3 hours while the app is running.",
                          style: GoogleFonts.outfit(
                            color: AppColors.textLow,
                            fontSize: 12,
                          ),
                        ),
                        value: _stressUpdates,
                        activeThumbColor: AppColors.accent,
                        activeTrackColor: AppColors.accentDim,
                        onChanged: (v) async {
                          setState(() => _stressUpdates = v);
                          await AppSettings.setStressUpdatesEnabled(v);
                        },
                      ),
                    ),
                    const SizedBox(height: 20),
                    Text(
                      "Plant alerts fire immediately when a reading requires action. Stress updates are periodic summaries of the current stress level.",
                      style: GoogleFonts.outfit(
                        fontSize: 12,
                        color: AppColors.textLow,
                      ),
                    ),
                  ],
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  PROFILE SETTINGS PAGE
// ═══════════════════════════════════════════════════════════════════════════════

class ProfileSettingsPage extends StatefulWidget {
  const ProfileSettingsPage({
    super.key,
    required this.plants,
    required this.selectedPlant,
  });

  final List<String> plants;
  final String? selectedPlant;

  @override
  State<ProfileSettingsPage> createState() => _ProfileSettingsPageState();
}

class _ProfileSettingsPageState extends State<ProfileSettingsPage> {
  late List<String> plants = [...widget.plants];
  late String? selectedPlant = widget.selectedPlant;

  String humidityPref = "mid";
  String temperaturePref = "mid";
  String soilPref = "mid";
  String lightPref = "mid";

  @override
  void initState() {
    super.initState();
    if (plants.isEmpty) loadPlants();
    loadPlantSettings();
  }

  Future<void> loadPlants() async {
    final res = await supabase.from('plant_settings').select('plant_label');
    final list = (res as List).map((e) => e['plant_label'] as String).toList();
    if (!mounted) return;
    setState(() {
      plants = list;
      if (!plants.contains(selectedPlant)) selectedPlant = null;
    });
  }

  void showTopMessage(String message, Color color) {
    final overlay = Overlay.of(context);
    final entry = OverlayEntry(
      builder: (_) => Positioned(
        top: 50,
        left: 20,
        right: 20,
        child: Material(
          color: Colors.transparent,
          child: Container(
            padding: const EdgeInsets.all(16),
            decoration: BoxDecoration(
              color: color,
              borderRadius: BorderRadius.circular(12),
              border: Border.all(color: color.withOpacity(0.5)),
              boxShadow: [
                BoxShadow(
                  color: color.withOpacity(0.3),
                  blurRadius: 20,
                  offset: const Offset(0, 4),
                ),
              ],
            ),
            child: Text(
              message,
              style: GoogleFonts.outfit(
                color: Colors.white,
                fontWeight: FontWeight.w600,
                fontSize: 14,
              ),
            ),
          ),
        ),
      ),
    );
    overlay.insert(entry);
    Future.delayed(const Duration(seconds: 2), entry.remove);
  }

  String normalize(String? v) {
    switch (v) {
      case "pLow":
        return "low";
      case "pMid":
        return "mid";
      case "pHigh":
        return "high";
      default:
        return "mid";
    }
  }

  String toDbValue(String v) {
    switch (v) {
      case "low":
        return "pLow";
      case "mid":
        return "pMid";
      case "high":
        return "pHigh";
      default:
        return "pMid";
    }
  }

  Future<void> loadPlantSettings() async {
    if (selectedPlant == null) return;
    final res = await supabase
        .from('plant_settings')
        .select()
        .eq('plant_label', selectedPlant!)
        .limit(1);
    if (res.isEmpty) return;
    final row = res[0];
    if (!mounted) return;
    setState(() {
      humidityPref = normalize(row['humidity_preference']);
      temperaturePref = normalize(row['temperature_preference']);
      soilPref = normalize(row['soil_preference']);
      lightPref = normalize(row['light_preference']);
    });
  }

  Future<void> savePlantSettings() async {
    if (selectedPlant == null) {
      showTopMessage("No plant selected", AppColors.high);
      return;
    }
    try {
      final current = await supabase
          .from('plant_settings')
          .select('version')
          .eq('plant_label', selectedPlant!.trim())
          .single();

      final int newVersion = ((current['version'] ?? 0) as num).toInt() + 1;

      final response = await supabase
          .from('plant_settings')
          .update({
            'humidity_preference': toDbValue(humidityPref),
            'temperature_preference': toDbValue(temperaturePref),
            'soil_preference': toDbValue(soilPref),
            'light_preference': toDbValue(lightPref),
            'version': newVersion,
          })
          .eq('plant_label', selectedPlant!.trim())
          .select();

      if (!mounted) return;

      if (response.isEmpty) {
        showTopMessage(
          "Nothing updated. Check plant selection.",
          AppColors.moderate,
        );
        return;
      }
      showTopMessage("Settings saved successfully", AppColors.healthy);
    } catch (_) {
      if (!mounted) return;
      showTopMessage("Failed to save settings", AppColors.high);
    }
  }

  Widget buildPlantSelector() {
    final plantValue = plants.contains(selectedPlant) ? selectedPlant : null;
    return DropdownButtonFormField<String>(
      key: ValueKey("settings-plant-$plantValue"),
      initialValue: plantValue,
      decoration: const InputDecoration(labelText: "Plant"),
      hint: Text(
        "Select a plant",
        style: GoogleFonts.outfit(color: AppColors.textLow, fontSize: 14),
      ),
      items: plants
          .map(
            (p) => DropdownMenuItem(
              value: p,
              child: Text(
                p,
                style: GoogleFonts.outfit(color: AppColors.textMid),
              ),
            ),
          )
          .toList(),
      onChanged: (v) {
        setState(() => selectedPlant = v);
        loadPlantSettings();
      },
    );
  }

  Widget buildPreferenceDropdown(
    String lbl,
    String value,
    ValueChanged<String?> onChanged,
  ) {
    return DropdownButtonFormField<String>(
      key: ValueKey("preference-$lbl-$value"),
      initialValue: ["low", "mid", "high"].contains(value) ? value : "mid",
      decoration: InputDecoration(labelText: lbl),
      items: [
        DropdownMenuItem(
          value: "low",
          child: Text(
            "Low",
            style: GoogleFonts.outfit(color: AppColors.textMid),
          ),
        ),
        DropdownMenuItem(
          value: "mid",
          child: Text(
            "Mid",
            style: GoogleFonts.outfit(color: AppColors.textMid),
          ),
        ),
        DropdownMenuItem(
          value: "high",
          child: Text(
            "High",
            style: GoogleFonts.outfit(color: AppColors.textMid),
          ),
        ),
      ],
      onChanged: onChanged,
    );
  }

  void goBack() => Navigator.of(context).pop(selectedPlant);

  @override
  Widget build(BuildContext context) {
    final isWide = MediaQuery.sizeOf(context).width >= 700;

    return Scaffold(
      appBar: AppBar(
        leading: IconButton(
          tooltip: "Back",
          onPressed: goBack,
          icon: const Icon(CupertinoIcons.back),
        ),
        title: const Text("Plant Preferences"),
        bottom: PreferredSize(
          preferredSize: const Size.fromHeight(1),
          child: Container(height: 1, color: AppColors.border),
        ),
      ),
      body: Stack(
        children: [
          Positioned.fill(child: CustomPaint(painter: _GridPainter())),
          SafeArea(
            child: Center(
              child: ConstrainedBox(
                constraints: const BoxConstraints(maxWidth: 720),
                child: ListView(
                  padding: const EdgeInsets.all(16),
                  children: [
                    buildPlantSelector(),
                    const SizedBox(height: 20),
                    Text(
                      "PREFERENCES",
                      style: GoogleFonts.outfit(
                        fontSize: 11,
                        fontWeight: FontWeight.w500,
                        color: AppColors.textLow,
                        letterSpacing: 1.8,
                      ),
                    ),
                    const SizedBox(height: 12),
                    if (isWide)
                      Row(
                        children: [
                          Expanded(
                            child: buildPreferenceDropdown(
                              "Humidity",
                              humidityPref,
                              (v) {
                                if (v != null) setState(() => humidityPref = v);
                              },
                            ),
                          ),
                          const SizedBox(width: 12),
                          Expanded(
                            child: buildPreferenceDropdown(
                              "Temperature",
                              temperaturePref,
                              (v) {
                                if (v != null)
                                  setState(() => temperaturePref = v);
                              },
                            ),
                          ),
                        ],
                      )
                    else ...[
                      buildPreferenceDropdown("Humidity", humidityPref, (v) {
                        if (v != null) setState(() => humidityPref = v);
                      }),
                      const SizedBox(height: 12),
                      buildPreferenceDropdown("Temperature", temperaturePref, (
                        v,
                      ) {
                        if (v != null) setState(() => temperaturePref = v);
                      }),
                    ],
                    const SizedBox(height: 12),
                    if (isWide)
                      Row(
                        children: [
                          Expanded(
                            child: buildPreferenceDropdown(
                              "Soil Moisture",
                              soilPref,
                              (v) {
                                if (v != null) setState(() => soilPref = v);
                              },
                            ),
                          ),
                          const SizedBox(width: 12),
                          Expanded(
                            child: buildPreferenceDropdown("Light", lightPref, (
                              v,
                            ) {
                              if (v != null) setState(() => lightPref = v);
                            }),
                          ),
                        ],
                      )
                    else ...[
                      buildPreferenceDropdown("Soil Moisture", soilPref, (v) {
                        if (v != null) setState(() => soilPref = v);
                      }),
                      const SizedBox(height: 12),
                      buildPreferenceDropdown("Light", lightPref, (v) {
                        if (v != null) setState(() => lightPref = v);
                      }),
                    ],
                    const SizedBox(height: 28),
                    FilledButton.icon(
                      onPressed: savePlantSettings,
                      icon: const Icon(Icons.save, size: 16),
                      label: const Text("Save Settings"),
                    ),
                    const SizedBox(height: 10),
                    OutlinedButton.icon(
                      onPressed: goBack,
                      icon: const Icon(CupertinoIcons.chevron_back, size: 16),
                      label: const Text("Back"),
                    ),
                    const SizedBox(height: 16),
                  ],
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  DATA TABLE PAGE
// ═══════════════════════════════════════════════════════════════════════════════

class DataTablePage extends StatefulWidget {
  const DataTablePage({
    super.key,
    required this.plants,
    required this.initialPlant,
  });

  final List<String> plants;
  final String? initialPlant;

  @override
  State<DataTablePage> createState() => _DataTablePageState();
}

class _DataTablePageState extends State<DataTablePage> {
  late List<String> _selectedPlants;
  List<String> _selectedColumns = ['soil_moisture_pct'];
  String _timeRange = '24h';
  List<Map<String, dynamic>> _rows = [];
  bool _loading = false;

  static const _allColumns = [
    ('soil_moisture_pct', 'Soil %'),
    ('temperature_c', 'Temp °C'),
    ('humidity_pct', 'Humidity %'),
    ('light_level_pct', 'Light %'),
    ('risk_class', 'Risk'),
  ];

  @override
  void initState() {
    super.initState();
    _selectedPlants =
        widget.initialPlant != null ? [widget.initialPlant!] : [];
    if (_selectedPlants.isNotEmpty) _fetchData();
  }

  Future<void> _fetchData() async {
    if (_selectedPlants.isEmpty || _selectedColumns.isEmpty) return;
    setState(() => _loading = true);

    final cols = ['plant_label', 'timestamp', ..._selectedColumns].join(',');
    final List<Map<String, dynamic>> all = [];

    for (final plant in _selectedPlants) {
      final res = await supabase
          .from('plant_readings')
          .select(cols)
          .eq('plant_label', plant)
          .order('timestamp', ascending: false)
          .limit(200);
      all.addAll(List<Map<String, dynamic>>.from(res));
    }

    final now = DateTime.now();
    final filtered = all.where((row) {
      final t = DateTime.parse(row['timestamp']);
      if (_timeRange == '24h') return now.difference(t).inHours <= 24;
      if (_timeRange == '7d') return now.difference(t).inDays <= 7;
      return true;
    }).toList();

    filtered.sort((a, b) => DateTime.parse(b['timestamp'])
        .compareTo(DateTime.parse(a['timestamp'])));

    if (!mounted) return;
    setState(() {
      _rows = filtered;
      _loading = false;
    });
  }

  String _fmtTimestamp(String ts) {
    final t = DateTime.parse(ts);
    return "${t.day.toString().padLeft(2, '0')}/${t.month.toString().padLeft(2, '0')} "
        "${t.hour.toString().padLeft(2, '0')}:${t.minute.toString().padLeft(2, '0')}";
  }

  String _fmtValue(dynamic v, String col) {
    if (v == null) return '—';
    if (col == 'risk_class') {
      return switch (v) {
        0 => 'Healthy',
        1 => 'Moderate',
        2 => 'High',
        _ => '$v',
      };
    }
    return (v as num).toStringAsFixed(1);
  }

  Color _riskColor(dynamic v) => switch (v) {
        0 => AppColors.healthy,
        1 => AppColors.moderate,
        2 => AppColors.high,
        _ => AppColors.textLow,
      };

  bool get _canFetch =>
      _selectedPlants.isNotEmpty && _selectedColumns.isNotEmpty;

  @override
  Widget build(BuildContext context) {
    final showPlantCol = _selectedPlants.length > 1;

    return Scaffold(
      appBar: AppBar(
        leading: IconButton(
          tooltip: "Back",
          onPressed: () => Navigator.of(context).pop(),
          icon: const Icon(CupertinoIcons.back),
        ),
        title: const Text("Data Table"),
        bottom: PreferredSize(
          preferredSize: const Size.fromHeight(1),
          child: Container(height: 1, color: AppColors.border),
        ),
      ),
      body: Stack(
        children: [
          Positioned.fill(child: CustomPaint(painter: _GridPainter())),
          SafeArea(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                // ── Filters ──────────────────────────────────────────────
                Container(
                  color: AppColors.bg,
                  padding: const EdgeInsets.fromLTRB(16, 12, 16, 12),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        "PLANTS",
                        style: GoogleFonts.outfit(
                          fontSize: 10,
                          fontWeight: FontWeight.w500,
                          color: AppColors.textLow,
                          letterSpacing: 1.6,
                        ),
                      ),
                      const SizedBox(height: 8),
                      Wrap(
                        spacing: 8,
                        runSpacing: 6,
                        children: widget.plants.map((p) {
                          final sel = _selectedPlants.contains(p);
                          return FilterChip(
                            label: Text(p),
                            selected: sel,
                            onSelected: (v) => setState(() {
                              if (v) {
                                _selectedPlants.add(p);
                              } else {
                                _selectedPlants.remove(p);
                              }
                            }),
                          );
                        }).toList(),
                      ),
                      const SizedBox(height: 12),
                      Text(
                        "FEATURES",
                        style: GoogleFonts.outfit(
                          fontSize: 10,
                          fontWeight: FontWeight.w500,
                          color: AppColors.textLow,
                          letterSpacing: 1.6,
                        ),
                      ),
                      const SizedBox(height: 8),
                      Wrap(
                        spacing: 8,
                        runSpacing: 6,
                        children: _allColumns.map((c) {
                          final sel = _selectedColumns.contains(c.$1);
                          return FilterChip(
                            label: Text(c.$2),
                            selected: sel,
                            onSelected: (v) => setState(() {
                              if (v) {
                                _selectedColumns.add(c.$1);
                              } else {
                                _selectedColumns.remove(c.$1);
                              }
                            }),
                          );
                        }).toList(),
                      ),
                      const SizedBox(height: 12),
                      Text(
                        "TIME RANGE",
                        style: GoogleFonts.outfit(
                          fontSize: 10,
                          fontWeight: FontWeight.w500,
                          color: AppColors.textLow,
                          letterSpacing: 1.6,
                        ),
                      ),
                      const SizedBox(height: 8),
                      Wrap(
                        spacing: 8,
                        runSpacing: 6,
                        children: const [
                          ('24h', 'Past 24 hours'),
                          ('7d', 'Past 7 days'),
                          ('all', 'All time'),
                        ].map((r) {
                          return ChoiceChip(
                            label: Text(r.$2),
                            selected: _timeRange == r.$1,
                            onSelected: (_) =>
                                setState(() => _timeRange = r.$1),
                          );
                        }).toList(),
                      ),
                      const SizedBox(height: 12),
                      SizedBox(
                        width: double.infinity,
                        child: FilledButton.icon(
                          onPressed: _canFetch ? _fetchData : null,
                          icon: const Icon(Icons.refresh, size: 16),
                          label: const Text("Load Data"),
                        ),
                      ),
                    ],
                  ),
                ),
                Container(height: 1, color: AppColors.border),

                // ── Table ─────────────────────────────────────────────────
                Expanded(
                  child: _loading
                      ? const Center(
                          child: CircularProgressIndicator(
                            color: AppColors.accent,
                          ),
                        )
                      : _rows.isEmpty
                          ? Center(
                              child: Text(
                                _canFetch
                                    ? "No data found"
                                    : "Select at least one plant and one column",
                                style: GoogleFonts.outfit(
                                  color: AppColors.textLow,
                                  fontSize: 13,
                                ),
                                textAlign: TextAlign.center,
                              ),
                            )
                          : SingleChildScrollView(
                              scrollDirection: Axis.vertical,
                              child: SizedBox(
                                width: double.infinity,
                                child: DataTable(
                                  headingRowColor: WidgetStateProperty.all(
                                    AppColors.surface,
                                  ),
                                  dataRowColor: WidgetStateProperty.all(
                                    Colors.transparent,
                                  ),
                                  dividerThickness: 1,
                                  columnSpacing: 20,
                                  headingTextStyle: GoogleFonts.outfit(
                                    color: AppColors.textLow,
                                    fontSize: 11,
                                    fontWeight: FontWeight.w600,
                                    letterSpacing: 1.2,
                                  ),
                                  dataTextStyle: GoogleFonts.outfit(
                                    color: AppColors.textMid,
                                    fontSize: 13,
                                  ),
                                  columns: [
                                    if (showPlantCol)
                                      const DataColumn(
                                        label: Text("PLANT"),
                                      ),
                                    const DataColumn(label: Text("TIME")),
                                    ..._selectedColumns.map((col) {
                                      final lbl = _allColumns
                                          .firstWhere((c) => c.$1 == col)
                                          .$2
                                          .toUpperCase();
                                      return DataColumn(label: Text(lbl));
                                    }),
                                  ],
                                  rows: _rows.map((row) {
                                    return DataRow(cells: [
                                      if (showPlantCol)
                                        DataCell(Text(
                                          row['plant_label'] ?? '—',
                                          style: GoogleFonts.outfit(
                                            color: AppColors.accent,
                                            fontSize: 13,
                                          ),
                                        )),
                                      DataCell(Text(
                                        _fmtTimestamp(row['timestamp']),
                                        style: GoogleFonts.outfit(
                                          color: AppColors.textLow,
                                          fontSize: 12,
                                        ),
                                      )),
                                      ..._selectedColumns.map((col) {
                                        final v = row[col];
                                        if (col == 'risk_class') {
                                          return DataCell(Text(
                                            _fmtValue(v, col),
                                            style: GoogleFonts.outfit(
                                              color: _riskColor(v),
                                              fontSize: 13,
                                              fontWeight: FontWeight.w600,
                                            ),
                                          ));
                                        }
                                        return DataCell(
                                          Text(_fmtValue(v, col)),
                                        );
                                      }),
                                    ]);
                                  }).toList(),
                                ),
                              ),
                            ),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}
