import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:supabase_flutter/supabase_flutter.dart';

const String supabaseUrl = 'https://yjjpgvsycxlaqubvedoa.supabase.co';
const String supabaseAnonKey = 'sb_publishable_mf_gFIoOv0-tIu9bhe6fzw_-CYi7BTd';
int riskClass = 0;
String riskLabel = "UNKNOWN";

void main() async {
  WidgetsFlutterBinding.ensureInitialized();

  await Supabase.initialize(url: supabaseUrl, anonKey: supabaseAnonKey);

  runApp(const MyApp());
}

final supabase = Supabase.instance.client;

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    final platform = defaultTargetPlatform;
    final isDesktop =
        platform == TargetPlatform.windows ||
        platform == TargetPlatform.macOS ||
        platform == TargetPlatform.linux;

    return MaterialApp(
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(
          seedColor: Colors.green,
          brightness: Brightness.dark,
        ),
        platform: platform,
        useMaterial3: true,
        visualDensity: isDesktop
            ? VisualDensity.compact
            : VisualDensity.standard,
      ),
      home: const Dashboard(),
    );
  }
}

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
  String statusText = "Loading...";
  Color statusColor = Colors.grey;

  Future<void> loadPlants() async {
    final res = await supabase.from('plant_settings').select('plant_label');

    final list = (res as List).map((e) => e['plant_label'] as String).toList();

    if (!mounted) return;

    setState(() {
      plants = list;
      if (!plants.contains(selectedPlant)) {
        selectedPlant = null;
      }
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
          statusColor = Colors.green;
          statusText = "Plant is healthy";
          break;
        case 1:
          riskLabel = "MODERATE RISK";
          statusColor = Colors.orange;
          statusText = "Moderate risk detected";
          break;
        case 2:
          riskLabel = "HIGH RISK";
          statusColor = Colors.red;
          statusText = "High risk detected";
          break;
        default:
          riskLabel = "UNKNOWN";
          statusColor = Colors.grey;
          statusText = "No data";
      }

      recommendationText = actions.isEmpty
          ? "No actions required"
          : actions.join("\n");
    });
  }

  bool isInRange(DateTime t) {
    final now = DateTime.now();

    if (timeRange == "24h") {
      return now.difference(t).inHours <= 24;
    }

    if (timeRange == "7d") {
      return now.difference(t).inDays <= 7;
    }

    return true;
  }

  Future<void> loadGraph() async {
    if (selectedPlant == null) return;

    final response = await supabase
        .from('plant_readings')
        .select()
        .eq('plant_label', selectedPlant!)
        .order('timestamp');

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
        final value = (row[m] ?? 0).toDouble();
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
        return Colors.blue;
      case "temperature_c":
        return Colors.red;
      case "humidity_pct":
        return Colors.green;
      case "light_level_pct":
        return Colors.yellow;
      default:
        return Colors.white;
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
  }

  Future<void> openProfileSettings() async {
    final selected = await Navigator.of(context).push<String>(
      MaterialPageRoute(
        builder: (context) =>
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
      decoration: const InputDecoration(
        labelText: "Plant",
        border: OutlineInputBorder(),
      ),
      hint: const Text("Select a plant"),
      items: plants
          .map((p) => DropdownMenuItem(value: p, child: Text(p)))
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
      decoration: const InputDecoration(
        labelText: "Time range",
        border: OutlineInputBorder(),
      ),
      items: const [
        DropdownMenuItem(value: "24h", child: Text("Past 24 hours")),
        DropdownMenuItem(value: "7d", child: Text("Past 7 days")),
        DropdownMenuItem(value: "all", child: Text("All time")),
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
              if (v) {
                selectedMetrics.add(m);
              } else {
                selectedMetrics.remove(m);
              }
            });
            loadGraph();
          },
        );
      }).toList(),
    );
  }

  Widget buildLegend() {
    return Wrap(
      spacing: 12,
      runSpacing: 8,
      children: selectedMetrics.map((m) {
        return Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            Container(width: 10, height: 10, color: colorFor(m)),
            const SizedBox(width: 5),
            Text(label(m)),
          ],
        );
      }).toList(),
    );
  }

  Widget buildChart() {
    if (selectedPlant == null) {
      return const Center(child: Text("Select a plant to view readings"));
    }

    if (selectedMetrics.isEmpty) {
      return const Center(child: Text("Select at least one metric"));
    }

    return LineChart(
      LineChartData(
        minY: 0,
        maxY: 100,
        gridData: const FlGridData(show: true),
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
                if (i < 0 || i >= timestamps.length) {
                  return const Text("");
                }
                final t = timestamps[i];
                return Text("${t.day}/${t.month}");
              },
            ),
          ),
          leftTitles: const AxisTitles(
            sideTitles: SideTitles(showTitles: true, reservedSize: 40),
          ),
        ),
        lineBarsData: selectedMetrics.map((m) {
          return LineChartBarData(
            spots: graphData[m] ?? [],
            isCurved: true,
            barWidth: 2,
            color: colorFor(m),
          );
        }).toList(),
      ),
    );
  }

  Widget buildStatusCard() {
    return Container(
      width: double.infinity,
      padding: const EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: statusColor,
        borderRadius: BorderRadius.circular(12),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            "STATUS: $riskLabel",
            style: const TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
          ),
          const SizedBox(height: 5),
          Text(statusText),
          const SizedBox(height: 12),
          const Text(
            "Recommendation:",
            style: TextStyle(fontWeight: FontWeight.bold),
          ),
          const SizedBox(height: 6),
          Text(recommendationText),
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
        title: Text(
          selectedPlant ?? "Plant Dashboard",
          overflow: TextOverflow.ellipsis,
        ),
        actions: [
          Padding(
            padding: const EdgeInsets.only(right: 12),
            child: showSettingsLabel
                ? FilledButton.icon(
                    onPressed: openProfileSettings,
                    icon: const Icon(Icons.settings),
                    label: const Text("Settings"),
                  )
                : IconButton(
                    tooltip: "Settings",
                    onPressed: openProfileSettings,
                    icon: const Icon(Icons.settings),
                  ),
          ),
        ],
      ),
      body: SafeArea(
        child: Center(
          child: ConstrainedBox(
            constraints: BoxConstraints(maxWidth: contentWidth),
            child: ListView(
              padding: const EdgeInsets.all(12),
              children: [
                if (isWide)
                  Row(
                    children: [
                      Expanded(child: buildPlantSelector()),
                      const SizedBox(width: 12),
                      Expanded(child: buildTimeRangeSelector()),
                    ],
                  )
                else ...[
                  buildPlantSelector(),
                  const SizedBox(height: 12),
                  buildTimeRangeSelector(),
                ],
                const SizedBox(height: 12),
                buildMetricSelector(),
                const SizedBox(height: 12),
                buildLegend(),
                const SizedBox(height: 16),
                SizedBox(
                  height: isWide ? 420 : 320,
                  child: Padding(
                    padding: const EdgeInsets.only(right: 12),
                    child: buildChart(),
                  ),
                ),
                const SizedBox(height: 16),
                buildStatusCard(),
              ],
            ),
          ),
        ),
      ),
    );
  }
}

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
    if (plants.isEmpty) {
      loadPlants();
    }
    loadPlantSettings();
  }

  Future<void> loadPlants() async {
    final res = await supabase.from('plant_settings').select('plant_label');
    final list = (res as List).map((e) => e['plant_label'] as String).toList();

    if (!mounted) return;

    setState(() {
      plants = list;
      if (!plants.contains(selectedPlant)) {
        selectedPlant = null;
      }
    });
  }

  void showTopMessage(String message, Color color) {
    final overlay = Overlay.of(context);

    final entry = OverlayEntry(
      builder: (context) => Positioned(
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
              boxShadow: const [
                BoxShadow(color: Colors.black26, blurRadius: 10),
              ],
            ),
            child: Text(
              message,
              style: const TextStyle(
                color: Colors.white,
                fontWeight: FontWeight.bold,
              ),
            ),
          ),
        ),
      ),
    );

    overlay.insert(entry);

    Future.delayed(const Duration(seconds: 2), () {
      entry.remove();
    });
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
      showTopMessage("No plant selected", Colors.red);
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
          Colors.orange,
        );
        return;
      }

      showTopMessage("Settings saved successfully", Colors.green);
    } catch (_) {
      if (!mounted) return;
      showTopMessage("Failed to save settings", Colors.red);
    }
  }

  Widget buildPlantSelector() {
    final plantValue = plants.contains(selectedPlant) ? selectedPlant : null;

    return DropdownButtonFormField<String>(
      key: ValueKey("settings-plant-$plantValue"),
      initialValue: plantValue,
      decoration: const InputDecoration(
        labelText: "Plant",
        border: OutlineInputBorder(),
      ),
      hint: const Text("Select a plant"),
      items: plants
          .map((p) => DropdownMenuItem(value: p, child: Text(p)))
          .toList(),
      onChanged: (v) {
        setState(() => selectedPlant = v);
        loadPlantSettings();
      },
    );
  }

  Widget buildPreferenceDropdown(
    String label,
    String value,
    ValueChanged<String?> onChanged,
  ) {
    return DropdownButtonFormField<String>(
      key: ValueKey("preference-$label-$value"),
      initialValue: ["low", "mid", "high"].contains(value) ? value : "mid",
      decoration: InputDecoration(
        labelText: label,
        border: const OutlineInputBorder(),
      ),
      items: const [
        DropdownMenuItem(value: "low", child: Text("Low")),
        DropdownMenuItem(value: "mid", child: Text("Mid")),
        DropdownMenuItem(value: "high", child: Text("High")),
      ],
      onChanged: onChanged,
    );
  }

  void goBack() {
    Navigator.of(context).pop(selectedPlant);
  }

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
        title: const Text("Profile Settings"),
      ),
      body: SafeArea(
        child: Center(
          child: ConstrainedBox(
            constraints: const BoxConstraints(maxWidth: 720),
            child: ListView(
              padding: const EdgeInsets.all(16),
              children: [
                buildPlantSelector(),
                const SizedBox(height: 16),
                if (isWide)
                  Row(
                    children: [
                      Expanded(
                        child: buildPreferenceDropdown(
                          "Humidity",
                          humidityPref,
                          (v) {
                            if (v == null) return;
                            setState(() => humidityPref = v);
                          },
                        ),
                      ),
                      const SizedBox(width: 12),
                      Expanded(
                        child: buildPreferenceDropdown(
                          "Temperature",
                          temperaturePref,
                          (v) {
                            if (v == null) return;
                            setState(() => temperaturePref = v);
                          },
                        ),
                      ),
                    ],
                  )
                else ...[
                  buildPreferenceDropdown("Humidity", humidityPref, (v) {
                    if (v == null) return;
                    setState(() => humidityPref = v);
                  }),
                  const SizedBox(height: 12),
                  buildPreferenceDropdown("Temperature", temperaturePref, (v) {
                    if (v == null) return;
                    setState(() => temperaturePref = v);
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
                            if (v == null) return;
                            setState(() => soilPref = v);
                          },
                        ),
                      ),
                      const SizedBox(width: 12),
                      Expanded(
                        child: buildPreferenceDropdown("Light", lightPref, (v) {
                          if (v == null) return;
                          setState(() => lightPref = v);
                        }),
                      ),
                    ],
                  )
                else ...[
                  buildPreferenceDropdown("Soil Moisture", soilPref, (v) {
                    if (v == null) return;
                    setState(() => soilPref = v);
                  }),
                  const SizedBox(height: 12),
                  buildPreferenceDropdown("Light", lightPref, (v) {
                    if (v == null) return;
                    setState(() => lightPref = v);
                  }),
                ],
                const SizedBox(height: 24),
                FilledButton.icon(
                  onPressed: savePlantSettings,
                  icon: const Icon(Icons.save),
                  label: const Text("Save Settings"),
                ),
                const SizedBox(height: 12),
                OutlinedButton.icon(
                  onPressed: goBack,
                  icon: const Icon(CupertinoIcons.back),
                  label: const Text("Back"),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
