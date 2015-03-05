title: "c# PropertyGrid 使用心得"
id: 802
date: 2013-11-03 08:38:09
tags: 
categories: 
- COCOS2D-X
---

这两天打算做个我们游戏的数据编辑器,c#无疑是最佳的选择

<!--more-->

<!--nextpage-->

------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

1.子项的折叠

举个例子,比如我们游戏中的塔有3个等级,每一级都会有一个攻击力,这样我做一个父项为攻击力,展开后分别为3个等级的设置!

[![QQ截图20130930125628]({{BASE_PATH}}/images/94c8e5a0affd3306da8182c62ae1e28012952a6c.jpg)](http://blog.justbilt.com/wp-content/uploads/2013/09/QQ截图20130930125628.jpg)

&nbsp;

添加一个类为子项的属性:
<pre class="lang:c++ decode:true">///塔的等级属性
public class TowerLevelProperty
{
	public float level1 { set; get; }
	public float level2 { set; get; }
	public float level3 { set; get; }
	public override string ToString()
	{
		return level1 + " , " + level2 + " , " + level3;
	}
}</pre>
在根类中添加一个变量:
<pre class="lang:c++ decode:true">/// root元素
public class TowerProperty
{
	private TowerLevelProperty _diamage = new TowerLevelProperty();

	[TypeConverter(typeof(ExpandableObjectConverter)), CategoryAttribute("属性设置")]
	public TowerLevelProperty 攻击力
	{
		set { _diamage = value; }
		get { return _diamage; }
	}

	private TowerLevelProperty _radius = new TowerLevelProperty();

	[TypeConverter(typeof(ExpandableObjectConverter)), CategoryAttribute("属性设置")]
	public TowerLevelProperty 攻击范围
	{
		set { _radius = value; }
		get { return _radius; }
	}

}</pre>
上面代码中的中文 攻击力 和 攻击范围 会直接显示在属性表中,如果不是这样,我绝对不会选起中文的函数名的!

&nbsp;

2.下拉列表属性支持

使用vs的属性窗口的时候,好多属性的设值我们点一下下面就会有一个下拉列表,在之间选择十分方便怎么实现的呢?

1)使用枚举

[![QQ截图20130930132220]({{BASE_PATH}}/images/c8e93052bf6af8b8b3783a0e99c856f0d2f728e8.jpg)](http://blog.justbilt.com/wp-content/uploads/2013/09/QQ截图20130930132220.jpg)

使用枚举可以十分方便的实现下拉列表支持:
<pre class="lang:c++ decode:true">//创建枚举
public enum AttackType
{
	远程攻击,
	锁链攻击,
	自身范围攻击,
}

//在root属性类中添加枚举类型变量
public class TowerProperty
{
	[CategoryAttribute("基本设置"), DescriptionAttribute("塔攻击方式")]
	public AttackType 攻击类型 { set; get; }
}</pre>
2).自定义下拉列表
<pre class="lang:c++ decode:true">//定义一个属性的下拉列表
public class AttackTypeConverter: StringConverter 
{
	public override bool GetStandardValuesSupported(ITypeDescriptorContext context)  
	{  
		return true;//返回是否支持下拉列表
	}

	public override StandardValuesCollection GetStandardValues(ITypeDescriptorContext context)  
	{
		return new StandardValuesCollection(new string[] { "远程攻击", "锁链攻击", "自身范围攻击" });//返回列表中的值
	}
}  

//在root类中添加变量
public class TowerProperty
{
	public string _attackType;

	[TypeConverter(typeof(AttackTypeConverter)), CategoryAttribute("基本设置")]
	public string 攻击类型
	{
		set { _attackType = value; }
		get { return _attackType; }
	}
}</pre>
&nbsp;

&nbsp;

&nbsp;